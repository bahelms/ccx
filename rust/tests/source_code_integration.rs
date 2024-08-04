use std::io::{BufReader, Cursor};

#[test]
fn compile_simple_program() {
    let input = r#"
    int main(void) {
        return 101;
    }
    "#;
    let reader = BufReader::new(Cursor::new(input.to_string()));
    ccx::compile(reader, "integration_test_output.c".to_string(), 3);

    let expected_output = "\t.globl main\nmain:\n\tmovl $101, %eax\n\tret\n\t.section .note.GNU-stack,\"\",@progbits\n";
    let contents = std::fs::read_to_string("integration_test_output.s").expect("File not found");
    assert_eq!(contents, expected_output);

    std::fs::remove_file("integration_test_output.s")
        .expect("Test source file failed to be deleted");
}
