.PHONY: test

compiler: src/*.cpp src/codegen/*.cpp
	mkdir -p ../bin
	clang++ -Wall -std=c++2a -o ../bin/compiler $^

test: compiler
	../bin/compiler --exit

debug_test: main.cpp lexer.cpp
	clang++ -Wall -std=c++2a -g -o main_debug.out $^
	gdb main_debug.out -- --exit
