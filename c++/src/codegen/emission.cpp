#include <format>
#include <fstream>
#include <iostream>
#include <memory>

#include "emission.h"

namespace ASM {
std::string asm_filename(std::string filename) {
    return filename.substr(0, filename.find_last_of(".")) + ".s";
}

std::string
format_instructions(std::vector<std::unique_ptr<Instruction>> &instrs) {
    std::string formatted_instrs{};
    for (auto &instr : instrs) {
        formatted_instrs += std::format("\t{}\n", instr->to_string());
    }
    return formatted_instrs;
}

std::string format_func_def(std::unique_ptr<FunctionDef> fn_def) {
    return std::format("\t.globl {}\n{}:\n{}", fn_def->name(), fn_def->name(),
                       format_instructions(fn_def->instructions()));
}

std::string format_operand(Register op) { return "%eax"; }

void emit_code(Program &program, std::string source_filename) {
    std::ofstream file(asm_filename(source_filename));
    if (file) {
        file << format_func_def(program.fn_def());

        // add this as the last line on Linux to disable an executable stack
        file << "\t.section .note.GNU-stack,\"\",@progbits\n";
    }
};
} // namespace ASM
