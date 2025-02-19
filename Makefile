.PHONY: install test clean

COMPILER=bin/compiler
BENCHMARKER=bin/benchmark
FLAGS=-Wall -std=c++2a
DEPS_FLAGS=-MMD -MP
SRC=$(wildcard src/*.cpp src/codegen/*.cpp)
OBJ=$(SRC:.cpp=.o)
DEPS=$(OBJ:.o=.d)
OBJ_EXCEPT_MAIN=$(filter-out src/main.o, $(OBJ))
BENCHMARKS=$(wildcard benchmarks/*.cpp)

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
	mkdir -p bin

clean:
	rm -f $(OBJ) $(DEPS) $(BENCHMARKER)

debug_test: $(OBJ)
	clang++ $(FLAGS) $(DEPS_FLAGS) -g -o $@.out $(OBJ)
	gdb $@.out -- --exit

benchmark: compile_benchmarks
	bin/benchmark --benchmark_out=benchmarks/latest_benchmark.txt --benchmark_out_format=console

compile_benchmarks: $(COMPILER) $(BENCHMARKS)
	clang++ $(FLAGS) -o $(BENCHMARKER) $(OBJ_EXCEPT_MAIN) $(BENCHMARKS) -isystem ../../vendors/benchmark/include -L../../vendors/benchmark/build/src -lbenchmark -lpthread

-include $(DEPS)
