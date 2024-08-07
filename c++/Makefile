.PHONY: install all test clean

COMPILER=../bin/compiler
SRC=$(wildcard src/*.cpp src/codegen/*.cpp)
OBJ=$(SRC:.cpp=.o)

all: $(COMPILER)

# link object files into executable
$(COMPILER): $(OBJ)
	clang++ -Wall -std=c++2a -o $@ $(OBJ)

# build object files
%.o: %.cpp
	clang++ -Wall -std=c++2a -c $< -o $@

test: $(COMPILER)
	$< --exit

install:
	mkdir -p ../bin

clean:
	rm $(OBJ)

debug_test: $(OBJ)
	clang++ -Wall -std=c++2a -g -o $@.out $(OBJ)
	gdb $@.out -- --exit
