mod codegen;
mod lexer;
mod parser;
mod tacky;

use lexer::Lexer;
use parser::Parser;
use std::{io::BufReader, io::Read};
use tacky::Tacky;

const LEX: i32 = 0;
const PARSE: i32 = 1;
const TACKY: i32 = 2;
const CODEGEN: i32 = 3;

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

    let mut tacky = Tacky::new(ast);
    let tacky_ir = tacky.convert_ast();

    // let asm = codegen::generate_assembly(ast);
    // if stage == CODEGEN {
    //     return;
    // }

    // codegen::emit_code(asm, filename);
}
