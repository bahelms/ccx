use super::assembly::{Asm, FunctionDef};
use std::fs::File;
use std::io::Write;

pub fn emit_code(assembly: Asm, filename: String) {
    let mut file = File::create(asm_filename(filename)).expect("Asm source file failed to open");
    file.write(format_func_def(assembly.func_def).as_bytes());
}

fn format_func_def(fn_def: FunctionDef) -> String {
    format!("\t.globl {}\n{}:\n[]", fn_def.name, fn_def.name)
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
