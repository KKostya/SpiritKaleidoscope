/////////////////////////////////////////////////////
//  Boost::static_visitor-based codegenerator 
//  based on boost::variant-based ast.
//  
//  Author: Kanishchev Konstantin
//  GitHub: github.com/KKostya/SpiritKaleidoscope
/////////////////////////////////////////////////////

#include <iostream>

#include "codegenerator.hpp"

#include "llvm/DerivedTypes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Target/TargetSelect.h"

inline llvm::Value * CgError(const std::string & msg, const std::string & name)
{ std::cout << "Error: " << msg << " \"" << name << "\"\n"; return 0; }

inline llvm::Function * FCgError(const std::string & msg, const std::string & name)
{ std::cout << "Error: " << msg << " \"" << name << "\"\n"; return 0; }


//////////////////////////////////////////////////////////
// Pimpl idiom, constructor and variable supply methods //
//////////////////////////////////////////////////////////
struct ExprCodegen::Impl
{
    llvm::Module * module;
    llvm::IRBuilder<> builder; 
    std::map<std::string,llvm::Value *> namedValues;
    Impl(llvm::Module * m):builder(llvm::getGlobalContext()),module(m){}
};

ExprCodegen::ExprCodegen(llvm::Module * module):pimpl(new Impl(module)) { }
void ExprCodegen::AddVariable(const std::string & var, llvm::Value * val)
{
    //!!!!!!!!!! Check for duplicates?
    pimpl->namedValues[var] = val;
}

///////////////////////
// Visitor operators //
///////////////////////

// Generator for double value 
llvm::Value * ExprCodegen::operator()(double n) const
{
    return llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(n));
}

// Generator for variable (string)
llvm::Value * ExprCodegen::operator()(const std::string & s)    const
{
    llvm::Value * v = pimpl->namedValues[s];
    if(!v) return CgError("unknown variable",s);
    return v;
}

// Generator for binary operation
llvm::Value * ExprCodegen::operator()(const ast::Binary & node) const
{
    llvm::Value * l = boost::apply_visitor(*this, node.lhs);
    llvm::Value * r = boost::apply_visitor(*this, node.rhs);
    if (l == 0 || r == 0) return 0;

    switch (node.op) 
    {
        case '+': return pimpl->builder.CreateFAdd(l, r, "addtmp");
        case '-': return pimpl->builder.CreateFSub(l, r, "subtmp");
        case '*': return pimpl->builder.CreateFMul(l, r, "multmp");
        case '/': return pimpl->builder.CreateFDiv(l, r, "divtmp");
        default : return CgError("(internal) unknown operator", std::string(&node.op,1));
    }
}

// Function call
llvm::Value * ExprCodegen::operator()(const ast::Call   & call) const
{
    llvm::Function * calleeF = pimpl->module->getFunction(call.callee);
    if(!calleeF) return CgError("unknown function",call.callee);

    // If argument mismatch error.
    if (calleeF->arg_size() != call.args.size()) 
        return CgError("incorrect # arguments passed into",call.callee);

    std::vector<llvm::Value*> argVals;
    for (unsigned i = 0, e = call.args.size(); i != e; ++i) 
    {
        llvm::Value * arg = boost::apply_visitor(*this,call.args[i]);
        if(arg == 0) return 0;
        argVals.push_back(arg);
    }

    return pimpl->builder.CreateCall(calleeF, argVals.begin(), argVals.end(), "calltmp");
}

//////////////////////////////////////////////////////////
/////////////////// Function CG Part /////////////////////
//////////////////////////////////////////////////////////

ProgCodegen::ProgCodegen(llvm::Module * mod):module(mod)
{
	llvm::InitializeNativeTarget();

    std::string ErrStr;
    engine = llvm::EngineBuilder(module).setErrorStr(&ErrStr).create();
    if (!engine) 
    {
        std::cerr <<  "Could not create ExecutionEngine:" << ErrStr << "\n";
        exit(1);
    }
}

// Prototype (declaration) generator
llvm::Function * ProgCodegen::operator()(const ast::Prototype & proto) const
{
    std::vector<const llvm::Type*> doubles(proto.args.size(),llvm::Type::getDoubleTy(llvm::getGlobalContext()));

    llvm::FunctionType * FT = llvm::FunctionType::get   (llvm::Type::getDoubleTy(llvm::getGlobalContext()),doubles, false);
    llvm::Function     * F  = llvm::Function    ::Create(FT, llvm::Function::ExternalLinkage, proto.name, module);

    // No redefinitions or redeclarations
    if(F->getName() != proto.name) 
    { 
        F->eraseFromParent(); 
        F = module->getFunction(proto.name); 
        if(!F->empty()) 
            return FCgError("redefinition of a function", proto.name);
        if(F->arg_size() != proto.args.size()) 
            return FCgError("redefinition of a function with different # args",proto.name);
    }

    return F;
}

// Function prototype+body (definition) generator
llvm::Function * ProgCodegen::operator()(const ast::Func & func)  const
{
    llvm::Function * funcVal = (*this)(func.proto); 
    if (funcVal == 0) return 0;

    //Creating ExprCodegen to process the function body.
    ExprCodegen expCG(module);

    //Supplying ExprCodegen with variables
    unsigned i = 0;
    for (llvm::Function::arg_iterator ai = funcVal->arg_begin(); i != func.proto.args.size(); ++ai, ++i) 
    {
        ai->setName(func.proto.args[i]);
        expCG.AddVariable(func.proto.args[i],ai);
    }

    //Building
    llvm::BasicBlock * BB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", funcVal);
    expCG.pimpl->builder.SetInsertPoint(BB);

    if(llvm::Value * bodyVal = boost::apply_visitor(expCG,func.body)) 
    {
        expCG.pimpl->builder.CreateRet(bodyVal);
        llvm::verifyFunction(*funcVal);
        return funcVal;
    }

    funcVal->eraseFromParent();
    return 0;
}

// Generator for toplevel expression
// It also evaluates the generated function
llvm::Function * ProgCodegen::operator()(const ast::Expr      & expr)  const 
{
    std::vector<const llvm::Type*> doubles(0,llvm::Type::getDoubleTy(llvm::getGlobalContext()));
    llvm::FunctionType * FT       = llvm::FunctionType::get   (llvm::Type::getDoubleTy(llvm::getGlobalContext()),doubles, false);
    llvm::Function     * funcVal  = llvm::Function    ::Create(FT, llvm::Function::ExternalLinkage,"", module);
    if (funcVal == 0) return 0;

    //Creating ExprCodegen to process the function body.
    ExprCodegen expCG(module);
    //Building
    llvm::BasicBlock * BB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", funcVal);
    expCG.pimpl->builder.SetInsertPoint(BB);

    if(llvm::Value * bodyVal = boost::apply_visitor(expCG,expr)) 
    {
        expCG.pimpl->builder.CreateRet(bodyVal);
        llvm::verifyFunction(*funcVal);
        //Evaluation
        std::cout << "ready " << engine << "\n";
        void *FPtr = engine->getPointerToFunction(funcVal);
        double (*FP)() = (double (*)())FPtr;
        std::cout << "Evaluated to " << FP() << "\n";

        return funcVal;
    }

    funcVal->eraseFromParent();
    return 0;
}


