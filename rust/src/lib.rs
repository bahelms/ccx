mod codegen;
mod lexer;
mod parser;

use lexer::Lexer;
use parser::Parser;
use std::{io::BufReader, io::Read};

const LEX: i32 = 0;
const PARSE: i32 = 1;
const CODEGEN: i32 = 2;

pub fn compile<T: Read>(reader: BufReader<T>, filename: String, stage: i32) {
    let mut lexer = Lexer::new(reader);
    let tokens = lexer.generate_tokens();
    if stage == LEX {
        for token in tokens {
            println!("{:?}", token);
        }
        return;
    }

    let mut parser = Parser::new(tokens);
    let ast = parser.parse();
    if stage == PARSE {
        println!("{}", ast);
        return;
    }

    let asm = codegen::generate_assembly(ast);
    if stage == CODEGEN {
        return;
    }

    codegen::emit_code(asm, filename);
}
