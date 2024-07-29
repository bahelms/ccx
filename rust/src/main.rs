mod codegen;
mod lexer;
mod parser;

use lexer::Lexer;
use parser::Parser;
use std::{fs::File, io::BufReader};

const LEX: i32 = 0;
const PARSE: i32 = 1;
const CODEGEN: i32 = 2;

fn main() {
    let filename = std::env::args().nth(1).expect("Error: No filename given");
    let stage: i32 = std::env::args()
        .nth(2)
        .expect("Error: No stage given")
        .parse()
        .expect("Error: Stage must be an integer");
    let file = File::open(&filename).expect("File not found");
    let reader = BufReader::new(file);

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
