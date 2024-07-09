compiler: main.cpp lexer.cpp
	clang++ -Wall -std=c++2a -o ../bin/compiler $^

test: compiler
	../bin/compiler --exit

