use super::assembly::{Asm, FunctionDef, Instruction};
use std::fs::File;
use std::io::Write;

pub fn emit_code(assembly: Asm, filename: String) {
    let mut file = File::create(asm_filename(filename)).expect("Asm source file failed to open");
    file.write(format_func_def(assembly.func_def).as_bytes())
        .expect("Write to asm source failed");
    file.write(b"\t.section .note.GNU-stack,\"\",@progbits\n")
        .expect("Write to asm source failed");
}

fn format_func_def(fn_def: FunctionDef) -> String {
    format!(
        "\t.globl {}\n{}:\n{}",
        fn_def.name,
        fn_def.name,
        format_instructions(fn_def.instructions)
    )
}

fn format_instructions(instrs: Vec<Instruction>) -> String {
    let mut formatted_instrs = String::new();
    for instr in instrs {
        formatted_instrs += &format!("\t{}\n", instr);
    }
    formatted_instrs
}

fn asm_filename(mut filename: String) -> String {
    filename.pop();
    filename.push('s');
    filename
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn asm_filename_changes_extension() {
        assert_eq!(asm_filename("hello.c".to_string()), "hello.s".to_string());
    }
}
