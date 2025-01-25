use crate::parser::Ast;

pub struct Tacky {
    ast: Ast,
}

impl Tacky {
    pub fn new(ast: Ast) -> Self {
        Self { ast }
    }

    pub fn convert_ast(&self) {}
}
