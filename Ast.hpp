/////////////////////////////////////////////////////
//  Basic AST tree based on boost::variant class
//  incorporates binary expression, funciton calls and definitions
//  
//  Author: Kanishchev Konstantin
//  GitHub: github.com/KKostya/SpiritKaleidoscope
/////////////////////////////////////////////////////


#ifndef AST_H
#define AST_H
#include <boost/config/warning_disable.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant.hpp>

#include <string>
#include <vector>

namespace ast
{
    ///////////////////////////////
    // Arithmetic expression ast //
    ///////////////////////////////
	struct Binary;
	struct Call;
	typedef boost::variant
		<
		  double, 				               // Number
		  std::string,				           // Variable
		  boost::recursive_wrapper<Binary>,    // Binary expression
		  boost::recursive_wrapper<Call>       // Function call 
		> 
		Expr;
	struct Binary     
	{  
		Expr lhs,rhs; 
		char op; 
		Binary(){}
		Binary(const Expr & l, const char o, const Expr & r):lhs(l),rhs(r),op(o){}
	};
	struct Call 	  { std::string callee; std::vector<Expr> args; };

    //////////////////////////
    // Language structs ast //
    //////////////////////////
	struct Prototype  { std::string   name; std::vector<std::string> args; };
	struct Func       { Prototype proto; Expr body; };

    ////////////////////////////
    // Generic expression ast //
    ////////////////////////////
	typedef boost::variant<Expr, Prototype, Func> Program;
}

BOOST_FUSION_ADAPT_STRUCT (ast::Binary   , (ast::Expr,      lhs   ) (char, op) (ast::Expr, rhs))
BOOST_FUSION_ADAPT_STRUCT (ast::Call     , (std::string,    callee) (std::vector<ast::Expr>, args))
BOOST_FUSION_ADAPT_STRUCT (ast::Prototype, (std::string,    name  ) (std::vector<std::string>, args))
BOOST_FUSION_ADAPT_STRUCT (ast::Func     , (ast::Prototype, proto ) (ast::Expr, body))

#endif
