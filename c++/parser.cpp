#include <iostream>

#include "parser.h"

Program Parser::parse() {
    for (auto token : _tokens) {
        std::cout << "Token: " << token.value() << std::endl;
    }
    Program ast;
    return ast;
}
