use crate::parser::{Ast, Exp, Function, Statement};
use core::fmt;

pub struct Asm {
    func_def: FunctionDef,
}

#[derive(Debug, PartialEq)]
enum Operand {
    Imm(String),
    Register,
}

impl fmt::Display for Operand {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Operand::Imm(val) => write!(f, "${}", val),
            Operand::Register => write!(f, "%eax"),
        }
    }
}

#[derive(Debug, PartialEq)]
enum Instruction {
    Mov(Operand, Operand),
    Ret,
}

impl fmt::Display for Instruction {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Instruction::Mov(src, dst) => write!(f, "movl {}, {}", src, dst),
            Instruction::Ret => write!(f, "ret"),
        }
    }
}

struct FunctionDef {
    name: String,
    instructions: Vec<Instruction>,
}

pub fn generate_assembly(ast: Ast) -> Asm {
    Asm {
        func_def: parse_func_def(ast.func),
    }
}

fn parse_func_def(func: Function) -> FunctionDef {
    FunctionDef {
        name: func.name,
        instructions: parse_instructions(func.body),
    }
}

fn parse_instructions(stmt: Statement) -> Vec<Instruction> {
    let mut instrs = Vec::new();
    match stmt {
        Statement::Return(exp) => {
            instrs.push(Instruction::Mov(parse_operand(exp), Operand::Register));
            instrs.push(Instruction::Ret);
        }
    }
    instrs
}

fn parse_operand(exp: Exp) -> Operand {
    match exp {
        Exp::Constant(c) => Operand::Imm(c),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn generating_a_simple_program() {
        let ast = Ast {
            func: Function {
                name: "main".to_string(),
                body: Statement::Return(Exp::Constant("789".to_string())),
            },
        };
        let asm = generate_assembly(ast);
        assert_eq!(asm.func_def.name, "main");
        assert_eq!(asm.func_def.instructions.len(), 2);
        assert_eq!(asm.func_def.instructions[0].to_string(), "movl $789, %eax");
        assert_eq!(asm.func_def.instructions[1].to_string(), "ret");
    }

    #[test]
    fn parsing_function_def_without_arguments() {
        let fn_def = parse_func_def(Function {
            name: "main".to_string(),
            body: Statement::Return(Exp::Constant("789".to_string())),
        });
        assert_eq!(fn_def.name, "main");
        assert_eq!(fn_def.instructions.len(), 2);
        assert_eq!(fn_def.instructions[0].to_string(), "movl $789, %eax");
        assert_eq!(fn_def.instructions[1].to_string(), "ret");
    }

    #[test]
    fn return_statements_produce_multiple_instructions() {
        let stmt = Statement::Return(Exp::Constant("789".to_string()));
        let instrs = parse_instructions(stmt);
        assert_eq!(instrs.len(), 2);
        assert_eq!(instrs[0].to_string(), "movl $789, %eax");
        assert_eq!(instrs[1].to_string(), "ret");
    }

    #[test]
    fn constants_generate_immediate_values() {
        let imm = parse_operand(Exp::Constant("42".to_string()));
        assert_eq!(imm, Operand::Imm("42".to_string()));
    }
}
