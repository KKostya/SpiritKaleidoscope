/////////////////////////////////////////////////////
//  Parser for arithmetic expressions -- realizing 
//  operator precedence parser algorithm in boost::spirit.
//  
//  Author: Kanishchev Konstantin
//  GitHub: github.com/KKostya/SpiritKaleidoscope
/////////////////////////////////////////////////////

#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_symbols.hpp> 

#include "arithmetic.hpp"

namespace parser{

template <typename Iterator> 
arithmetic<Iterator>::arithmetic():
    arithmetic::base_type(expr,"expression")
{
	namespace ph = boost::phoenix;
	using namespace qi::labels;
	using qi::double_;
	using qi::char_;
	using qi::omit;
	using qi::attr;
	
	  op.add ("+" ,'+') ("-" ,'-') ("*",'*') ("/",'/'); // operators
//	  op.add ("&&",'a') ("||",'o') (".",'.');
//	  op.add ("&",'&')  ("|",'|');

	prec.add ("+", 1) ("-", 1) ("*", 2) ("/", 2); // their precedences
//	prec.add ("&&",5) ("||",5) (".",6);
//	prec.add ("&",4)  ("|",4);

	identifier = qi::lexeme[char_("a-zA-Z") >> *char_("0-9a-zA-Z")];
    // This one is to handle function calls
	call = identifier >> '(' > (expr % ',') > ')';

    // Primary is either brackets or number or identifier
	primary = '(' >  expr > ')' | double_ | call | identifier ; 

    //Entry point -- read primary and recursively pass it up 
    // into rhs parser with zero precedence
	expr = omit[primary[_a=_1]] >> rhs(0,_a);
    
    // Rhs either  construct a binary or just returns second argument
	rhs  =  omit[ lookahead(_r1)[_a=_1] >> op[_b=_1] >> bin(_a)[_c=_1] ] 
            >> rhs(_r1,ph::construct<ast::Binary>(_r2,_b,_c))
	     |  attr(_r2);
    // Bin reads next primary and either recursively calls rhs with
    // increased precedence or just returns
	bin  =  omit[ primary[_a=_1] >> lookahead(_r1)[_b=_1] ] 
            >> rhs(_r1+1,_a) 
	     | primary;

    // Looahead checks if the precedence of the next 
    // operator is  leq than the current one
	lookahead = &(prec[ _pass = _r1<=(_val=_1) ]);

    ///////////////// Error hadling ////////////////
            op.name("operator");
	      expr.name("expression");
	       rhs.name("right hand side");
	       bin.name("binary expression");
	 lookahead.name("lookahead");
	   primary.name("primary expression");
	identifier.name("identifier");

    using qi::on_error;
    using qi::fail;
    on_error<fail>
        (
         expr 
         , std::cout
         << ph::val("Error! Expecting ")
         << _4                               // what failed?
         << ph::val(" here: \"")
         << ph::construct<std::string>(_3, _2)   // iterators to error-pos, end
         << ph::val("\"")
         << std::endl
        );

}

} // namespace parser
