/////////////////////////////////////////////////////
//  Kaleidoscope language toplevel exression parser.
//  
//  Author: Kanishchev Konstantin
//  GitHub: github.com/KKostya/SpiritKaleidoscope
/////////////////////////////////////////////////////

#ifndef KALEIDOSCOPE_HPP
#define KALEIDOSCOPE_HPP

#include "Ast.hpp"
#include "arithmetic.hpp"

namespace parser{

namespace qi = boost::spirit::qi;
namespace ns = boost::spirit::ascii;

template <typename Iterator>
struct kaleidoscope: qi::grammar<Iterator,ast::Program(),ns::space_type >
{
    arithmetic<Iterator> arithm;

	qi::rule<Iterator,ast::Program()  , ns::space_type> program;
	qi::rule<Iterator,ast::Prototype(), ns::space_type> proto;
	qi::rule<Iterator,ast::Func()     , ns::space_type> func;
	qi::rule<Iterator,std::string()> identifier;
	
    kaleidoscope();
};

}
#endif
