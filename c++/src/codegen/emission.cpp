#include <format>
#include <fstream>
#include <iostream>
#include <memory>

#include "emission.h"

namespace Asm {
std::string asm_filename(std::string filename) {
    return filename.substr(0, filename.find_last_of(".")) + ".s";
}

std::string fn_prologue() { return "\tpushq %rbp\n\tmovq %rsp, %rbp\n"; }
std::string fn_epilogue() { return "\tmovq %rbp, %rsp\n\tpopq %rbp\n"; }

std::string
format_instructions(std::vector<std::unique_ptr<Instruction>> &instrs) {
    std::string formatted_instrs{};
    for (auto &instr : instrs) {
        if (auto *ret = dynamic_cast<Ret *>(instr.get())) {
            formatted_instrs += fn_epilogue();
        }
        formatted_instrs += std::format("\t{}\n", instr->to_string());
    }
    return formatted_instrs;
}

std::string format_func_def(FunctionDef fn_def) {
    return std::format("\t.globl {}\n{}:\n{}{}", fn_def.name(), fn_def.name(),
                       fn_prologue(),
                       format_instructions(fn_def.instructions()));
}

void emit_code(Program &program, std::string source_filename) {
    std::ofstream file(asm_filename(source_filename));
    if (file) {
        file << format_func_def(program.fn_def());

        // add this as the last line on Linux to disable an executable stack
        file << "\t.section .note.GNU-stack,\"\",@progbits\n";
    }
};
} // namespace Asm
