#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "lexer.h"

int main(int argc, char *argv[]) {
    std::ifstream file(argv[1]);

    if (file) {
        Lexer lexer(file);
        const std::vector<Token> tokens = lexer.generate_tokens();

        for (auto token : tokens) {
            std::cout << "Token: " << token.value() << std::endl;
        }
    }
}
