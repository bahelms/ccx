.PHONY: install test clean

COMPILER=../bin/compiler
FLAGS=-Wall -std=c++2a -MMD -MP
SRC=$(wildcard src/*.cpp src/codegen/*.cpp)
OBJ=$(SRC:.cpp=.o)
DEPS=$(OBJ:.o=.d)

all: $(COMPILER)

# link object files into executable
$(COMPILER): $(OBJ)
	clang++ $(FLAGS) -o $@ $(OBJ)

# build object files
%.o: %.cpp
	clang++ $(FLAGS) -c $< -o $@

test: $(COMPILER)
	$< --exit

install:
	mkdir -p ../bin

clean:
	rm -f $(OBJ) $(DEPS)

debug_test: $(OBJ)
	clang++ -Wall -std=c++2a -g -o $@.out $(OBJ)
	gdb $@.out -- --exit

-include $(DEPS)
