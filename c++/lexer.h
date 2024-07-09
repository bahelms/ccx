#include <fstream>
#include <string>
#include <vector>

class Token {
    std::string _value;

  public:
    Token(std::string v) : _value(v){};
    std::string value() { return _value; }
};

class Lexer {
    std::ifstream &_stream;

  public:
    Lexer(std::ifstream &s) : _stream(s){};

    std::vector<Token> generate_tokens();
};
