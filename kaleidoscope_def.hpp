/////////////////////////////////////////////////////
//  Kaleidoscope language toplevel exression parser.
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

#include "kaleidoscope.hpp"

namespace parser{

template <typename Iterator> 
kaleidoscope<Iterator>::kaleidoscope():
    kaleidoscope::base_type(program,"program")
{
	using qi::char_;
	using qi::lit;

    identifier = qi::lexeme[char_("a-zA-Z") >> *char_("0-9a-zA-Z")];
    proto    = identifier > '(' >> -(identifier % ',') > ')';
    func     = proto > arithm;
    program  =   lit("extern") > proto      
               | lit("def")    > func 
               | arithm;

    // Error handling 
      proto.name("prototype");
       func.name("function definition");
    program.name("toplevel");

	namespace ph = boost::phoenix;
	using namespace qi::labels;
    using qi::on_error;
    using qi::fail;
    on_error<fail>
        (
         program 
         , std::cout
         << ph::val("Error! Expecting ")
         << _4                               // what failed?
         << ph::val(" here: \"")
         << ph::construct<std::string>(_3, _2)   // iterators to error-pos, end
         << ph::val("\"")
         << std::endl
        );
}

}
