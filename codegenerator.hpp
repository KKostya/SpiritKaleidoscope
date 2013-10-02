/////////////////////////////////////////////////////
//  Boost::static_visitor-based codegenerator 
//  based on boost::variant-based ast.
//  
//  Author: Kanishchev Konstantin
//  GitHub: github.com/KKostya/SpiritKaleidoscope
/////////////////////////////////////////////////////

#ifndef CODEGENERATOR_HPP
#define CODEGENERATOR_HPP

#include "Ast.hpp"

// Forward declarations
namespace llvm
{
    class Value;
    class Module;
    class Function;
    class ExecutionEngine;
}


class ExprCodegen : public boost::static_visitor<llvm::Value *>
{
    struct Impl;
    Impl * pimpl;
public:
    friend class ProgCodegen;

    ExprCodegen(llvm::Module * mod);
    void AddVariable(const std::string & var, llvm::Value * val);

    llvm::Value * operator()(double n) const;
    llvm::Value * operator()(const std::string & s)    const;
    llvm::Value * operator()(const ast::Binary & node) const;
    llvm::Value * operator()(const ast::Call   & call) const;
};


class ProgCodegen : public boost::static_visitor<llvm::Function *>
{
    llvm::Module * module;
    llvm::ExecutionEngine * engine;
public:
	ProgCodegen(llvm::Module * mod);
	llvm::Function * operator()(const ast::Prototype & proto) const; 
	llvm::Function * operator()(const ast::Func      & func)  const; 
	llvm::Function * operator()(const ast::Expr      & expr)  const; 
};

#endif

