use regex::Regex;
use std::{io::BufReader, io::Read};

#[derive(Debug, PartialEq)]
pub enum Token {
    Constant(String),
    Identifier(String),
    Literal(String), // Punctuation - can make these their own tokens (OpenBrace, etc)
}

pub struct Lexer<T> {
    reader: BufReader<T>,
    char_buffer: String,
    pub tokens: Vec<Token>,
    patterns: Patterns,
}

impl<T: Read> Lexer<T> {
    pub fn new(reader: BufReader<T>) -> Self {
        Self {
            reader,
            char_buffer: String::new(),
            tokens: Vec::new(),
            patterns: Patterns::new(),
        }
    }

    pub fn generate_tokens(&mut self) -> &Vec<Token> {
        let mut buffer = [0; 1];
        while let Ok(bytes_read) = self.reader.read(&mut buffer) {
            if bytes_read == 0 {
                break;
            }
            let ch = buffer[0] as char;
            match ch {
                ';' | '(' | ')' => {
                    self.flush_char_buffer();
                    self.tokens.push(Token::Literal(ch.to_string()));
                }
                '{' | '}' => {
                    self.tokens.push(Token::Literal(ch.to_string()));
                }
                _ => {
                    let ch_str = ch.to_string();
                    if self.patterns.whitespace.is_match(&ch_str) {
                        self.flush_char_buffer();
                    } else {
                        self.char_buffer.push(ch);
                    }
                }
            }
        }
        &self.tokens
    }

    fn flush_char_buffer(&mut self) {
        if !self.char_buffer.is_empty() {
            if self.patterns.constant.is_match(&self.char_buffer) {
                self.tokens.push(Token::Constant(self.char_buffer.clone()));
            } else if self.patterns.identifier.is_match(&self.char_buffer) {
                self.tokens
                    .push(Token::Identifier(self.char_buffer.clone()));
            } else {
                panic!("Identifiers can't begin with a digit: {}", self.char_buffer);
            }
            self.char_buffer.clear();
        }
    }
}

struct Patterns {
    whitespace: Regex,
    identifier: Regex,
    constant: Regex,
}

impl Patterns {
    fn new() -> Self {
        Self {
            identifier: Regex::new(r"^[a-zA-Z_]\w*$").unwrap(),
            constant: Regex::new(r"^[0-9]+$").unwrap(),
            whitespace: Regex::new(r"\s").unwrap(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::{BufReader, Cursor};

    #[test]
    #[should_panic]
    fn identifiers_cant_start_with_a_digit() {
        Lexer::new(init_reader("2foo;")).generate_tokens();
    }

    #[test]
    fn identifiers_can_have_digits() {
        let mut lexer = Lexer::new(init_reader("i2x6("));
        let tokens = lexer.generate_tokens();
        assert_eq!(tokens[0], Token::Identifier("i2x6".to_string()));
        assert_eq!(tokens[1], Token::Literal("(".to_string()));
    }

    #[test]
    fn constant_token() {
        let mut lexer = Lexer::new(init_reader("2246;"));
        let tokens = lexer.generate_tokens();
        assert_eq!(tokens[0], Token::Constant("2246".to_string()));
        assert_eq!(tokens[1], Token::Literal(";".to_string()));
    }

    #[test]
    fn whitespace_is_ignored() {
        let mut lexer = Lexer::new(init_reader(" \n\t ;  "));
        let tokens = lexer.generate_tokens();
        assert_eq!(tokens.len(), 1);
        assert_eq!(tokens[0], Token::Literal(";".to_string()));
    }

    #[test]
    fn gen_tokens_for_simple_valid_program() {
        let program = "int \tmain(void)     {\n return 42; \n}\n";
        let mut lexer = Lexer::new(init_reader(program));
        let tokens = lexer.generate_tokens();
        assert_eq!(tokens.len(), 10);
    }

    fn init_reader(input: &str) -> BufReader<Cursor<String>> {
        BufReader::new(Cursor::new(input.to_string()))
    }
}
