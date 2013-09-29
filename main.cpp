#include <iostream>
#include <string>

#include "arithmetic.hpp"
#include "kaleidoscope.hpp"
#include "codegenerator.hpp"

#include "llvm/LLVMContext.h"
#include "llvm/Value.h"
#include "llvm/Function.h"
#include "llvm/Module.h"

typedef std::string::iterator Iter;

int main()
{
    llvm::LLVMContext & Context = llvm::getGlobalContext();
    llvm::Module * module = new llvm::Module("my cool jit", Context);

	namespace qi = boost::spirit::qi;
	namespace ns = boost::spirit::ascii;

	parser::kaleidoscope<Iter> prog;
	std::string str;
	while (getline(std::cin, str))
	{
		ast::Program out;
		if (str[0] == 'q' || str[0] == 'Q') break;
		Iter i = str.begin();
		if (qi::phrase_parse(i,str.end(),prog,ns::space,out))
		{
			if(i == str.end())
            {
                std::cout << "-------------------------\nParsing succeeded\n";
                llvm::Function * v = boost::apply_visitor(ProgCodegen(module), out);
                if(v != 0) module->dump();
            }
		}
		else std::cout << "Parsing failed\n";

	}
	std::cout << "Bye... :-) \n\n";
	return 0;
}
