LLVM_CFLAGS=$(shell ../llvm_install/bin/llvm-config --cppflags)
LLVM_LINKFLAGS=$(shell ../llvm_install/bin/llvm-config --ldflags --libs core jit native)
BOOST=-I../boost_1_55_0/

test: arithmetic.o main.o kaleidoscope.o codegenerator.o 
	g++ $^ $(LLVM_LINKFLAGS) -ldl -lpthread -lcurses -o test

arithmetic.o: arithmetic.cpp arithmetic_def.hpp arithmetic.hpp
	g++ $(BOOST) -c $< 

kaleidoscope.o: kaleidoscope.cpp kaleidoscope_def.hpp kaleidoscope.hpp
	g++ $(BOOST) -c $< 

codegenerator.o: codegenerator.cpp codegenerator.hpp
	g++ $(LLVM_CFLAGS) $(BOOST) -c $<

main.o: main.cpp 
	g++ $(LLVM_CFLAGS) $(BOOST) -c $<
