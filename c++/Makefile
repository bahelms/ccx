.PHONY: install test clean

COMPILER=../bin/compiler
FLAGS=-Wall -std=c++2a
DEPS_FLAGS=-MMD -MP
SRC=$(wildcard src/*.cpp src/codegen/*.cpp)
OBJ=$(SRC:.cpp=.o)
DEPS=$(OBJ:.o=.d)
OBJ_EXCEPT_MAIN=$(filter-out src/main.o, $(OBJ))
BENCHMARKS=$(wildcard benchmarks/*)

all: $(COMPILER)

# link object files into executable
$(COMPILER): $(OBJ)
	clang++ $(FLAGS) $(DEPS_FLAGS) -o $@ $(OBJ)

# build object files
%.o: %.cpp
	clang++ $(FLAGS) $(DEPS_FLAGS) -c $< -o $@

test: $(COMPILER)
	$< --exit

install:
	mkdir -p ../bin

clean:
	rm -f $(OBJ) $(DEPS)

debug_test: $(OBJ)
	clang++ $(FLAGS) $(DEPS_FLAGS) -g -o $@.out $(OBJ)
	gdb $@.out -- --exit

benchmark: compile_benchmarks
	../bin/benchmark --benchmark_out=benchmarks.txt --benchmark_out_format=console

compile_benchmarks: $(COMPILER) $(BENCHMARKS)
	clang++ $(FLAGS) -o ../bin/benchmark $(OBJ_EXCEPT_MAIN) $(BENCHMARKS) -isystem ../../../vendors/benchmark/include -L../../../vendors/benchmark/build/src -lbenchmark -lpthread

-include $(DEPS)
