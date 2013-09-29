CFLAGS := `llvm-config --cppflags`
LINKFLAGS := `llvm-config --ldflags --libs core jit native`

BOOST := -I../../boost_1_48_0

test: arithmetic.o main.o kaleidoscope.o codegenerator.o 
	g++ $^ `llvm-config --ldflags --libs core jit native` -o test

arithmetic.o: arithmetic.cpp arithmetic_def.hpp arithmetic.hpp
	g++ $(BOOST) -c $< 

kaleidoscope.o: kaleidoscope.cpp kaleidoscope_def.hpp kaleidoscope.hpp
	g++ $(BOOST) -c $< 

codegenerator.o: codegenerator.cpp codegenerator.hpp
	g++ $(CFLAGS) $(BOOST) -c $<

main.o: main.cpp 
	g++ $(CFLAGS) $(BOOST) -c $<
