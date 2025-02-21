# CCX - a compiler for the C language
### Setup
    $ make install
    $ make

### Run the compiler
    $ ./ccx path_to_file.c

### Diplay lexical tokens
    $ ./ccx --lex path_to_file.c

### Diplay AST
    $ ./ccx --parse path_to_file.c

### Diplay TACKY IR
    $ ./ccx --tacky path_to_file.c

### Generate assembly file *.s
    $ ./ccx --codegen path_to_file.c

### Testing
    $ make test

### Run benchmarks
    $ make benchmark
