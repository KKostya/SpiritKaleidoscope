/////////////////////////////////////////////////////
//  Parser for arithmetic expressions -- realizing 
//  operator precedence parser algorithm in boost::spirit.
//  
//  Author: Kanishchev Konstantin
//  GitHub: github.com/KKostya/SpiritKaleidoscope
/////////////////////////////////////////////////////

#ifndef ARITHMETIC_HPP
#define ARITHMETIC_HPP

#include <boost/spirit/include/qi.hpp>
#include "Ast.hpp"

#include <map>


namespace parser{

namespace qi = boost::spirit::qi;
namespace ns = boost::spirit::ascii;
using ast::Expr;

template <typename Iterator>
struct arithmetic: qi::grammar<Iterator,Expr(),qi::locals<Expr>,ns::space_type >
{
	qi::rule<Iterator,Expr()        , qi::locals<Expr>         , ns::space_type> expr;
	qi::rule<Iterator,Expr(int,Expr), qi::locals<int,char,Expr>, ns::space_type> rhs;
	qi::rule<Iterator,Expr(int)     , qi::locals<Expr,int>     , ns::space_type> bin;
	qi::rule<Iterator,int (int)     ,                            ns::space_type> lookahead;
	qi::rule<Iterator,Expr()        ,                            ns::space_type> primary;
	qi::rule<Iterator,ast::Call()   ,                            ns::space_type> call;
	qi::rule<Iterator,std::string()> identifier;

	qi::symbols<char,char> op;
	qi::symbols<char,int> prec;
	
	arithmetic();
};

}

#endif
