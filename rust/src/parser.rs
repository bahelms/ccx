use crate::lexer::Token;
use core::fmt;

pub enum Exp {
    Constant(String),
}

impl fmt::Display for Exp {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Exp::Constant(v) => write!(f, "Constant({})", v),
        }
    }
}

enum Statement {
    Return(Exp),
}

impl fmt::Display for Statement {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Statement::Return(exp) => write!(f, "Return({})", exp),
        }
    }
}

struct Function {
    name: String,
    body: Statement,
}

impl fmt::Display for Function {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(
            f,
            "Function(\n  name=\"{}\",\n  body={}\n)",
            self.name, self.body
        )
    }
}

pub struct Ast {
    func: Function,
}

impl fmt::Display for Ast {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "Program(\n  {}\n)", self.func)
    }
}

pub struct Parser<'a> {
    tokens: &'a Vec<Token>,
    current_token: usize,
}

impl<'a> Parser<'a> {
    pub fn new(tokens: &'a Vec<Token>) -> Self {
        Self {
            tokens,
            current_token: 0,
        }
    }

    pub fn parse(&mut self) -> Ast {
        let func = self.parse_function();
        if self.current_token != self.tokens.len() {
            panic!(
                "Unexpected token found: {:?}",
                self.tokens[self.current_token]
            );
        }
        Ast { func }
    }

    fn parse_exp(&mut self) -> Exp {
        let token = &self.tokens[self.current_token];
        self.current_token += 1;
        match token {
            Token::Constant(v) => Exp::Constant(v.clone()),
            t => panic!("Not a constant: {:?}", t),
        }
    }

    fn parse_statement(&mut self) -> Statement {
        self.expect(Token::Identifier("return".to_string()));
        let exp = self.parse_exp();
        self.expect(Token::Literal(";".to_string()));
        Statement::Return(exp)
    }

    fn parse_function(&mut self) -> Function {
        self.expect(Token::Identifier("int".to_string()));
        let token = &self.tokens[self.current_token];
        self.current_token += 1;
        let name = match token {
            Token::Identifier(n) => n,
            n => panic!("Invalid function name: {:?}", n),
        };
        self.expect(Token::Literal("(".to_string()));
        self.expect(Token::Identifier("void".to_string()));
        self.expect(Token::Literal(")".to_string()));
        self.expect(Token::Literal("{".to_string()));
        let stmt = self.parse_statement();
        self.expect(Token::Literal("}".to_string()));
        Function {
            name: name.clone(),
            body: stmt,
        }
    }

    fn expect(&mut self, expected: Token) {
        if self.current_token == self.tokens.len() {
            panic!("Missing \"{:?}\"", expected);
        }
        let actual = &self.tokens[self.current_token];
        self.current_token += 1;
        if actual != &expected {
            panic!("Expected \"{:?}\" but got \"{:?}\"", expected, actual);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    #[should_panic]
    fn parse_with_extra_tokens() {
        let tokens = vec![
            ident("int"),
            ident("my_func"),
            lit("("),
            ident("void"),
            lit(")"),
            lit("{"),
            ident("return"),
            constant("420"),
            lit(";"),
            lit("}"),
            ident("foo"),
            ident("bar"),
        ];
        Parser::new(&tokens).parse();
    }

    #[test]
    fn parse_successfully() {
        let tokens = vec![
            ident("int"),
            ident("my_func"),
            lit("("),
            ident("void"),
            lit(")"),
            lit("{"),
            ident("return"),
            constant("420"),
            lit(";"),
            lit("}"),
        ];
        let mut parser = Parser::new(&tokens);
        let ast = parser.parse();
        assert_eq!(
            ast.to_string(),
            "Program(\n  Function(\n  name=\"my_func\",\n  body=Return(Constant(420))\n)\n)"
        );
    }

    #[test]
    #[should_panic]
    fn parse_function_with_missing_token() {
        let tokens = vec![
            ident("int"),
            ident("my_func"),
            ident("void"),
            lit(")"),
            lit("{"),
            ident("return"),
            constant("420"),
            lit(";"),
            lit("}"),
        ];
        let mut parser = Parser::new(&tokens);
        parser.parse_function();
    }

    #[test]
    #[should_panic]
    fn parse_function_with_invalid_name() {
        let tokens = vec![
            ident("int"),
            constant("3"),
            lit("("),
            ident("void"),
            lit(")"),
            lit("{"),
            ident("return"),
            constant("420"),
            lit(";"),
            lit("}"),
        ];
        let mut parser = Parser::new(&tokens);
        parser.parse_function();
    }

    #[test]
    fn parse_function_success() {
        let tokens = vec![
            ident("int"),
            ident("my_func"),
            lit("("),
            ident("void"),
            lit(")"),
            lit("{"),
            ident("return"),
            constant("420"),
            lit(";"),
            lit("}"),
        ];
        let mut parser = Parser::new(&tokens);
        let stmt = parser.parse_function();
        assert_eq!(
            stmt.to_string(),
            "Function(\n  name=\"my_func\",\n  body=Return(Constant(420))\n)"
        );
    }

    #[test]
    #[should_panic]
    fn parse_statement_with_missing_expression() {
        // TODO: `return;` is actually valid C
        let tokens = vec![ident("return"), lit(";")];
        let mut parser = Parser::new(&tokens);
        parser.parse_statement();
    }

    #[test]
    #[should_panic]
    fn parse_statement_with_missing_semicolon() {
        let tokens = vec![ident("bork"), constant("1234")];
        let mut parser = Parser::new(&tokens);
        parser.parse_statement();
    }

    #[test]
    #[should_panic]
    fn parse_statement_with_missing_identifier() {
        let tokens = vec![ident("bork"), constant("1234"), lit(";")];
        let mut parser = Parser::new(&tokens);
        parser.parse_statement();
    }

    #[test]
    fn parse_statement_success() {
        let tokens = vec![ident("return"), constant("1234"), lit(";")];
        let mut parser = Parser::new(&tokens);
        let stmt = parser.parse_statement();
        assert_eq!(stmt.to_string(), "Return(Constant(1234))");
    }

    #[test]
    #[should_panic]
    fn parse_exp_with_error() {
        let tokens = vec![Token::Identifier("bark".to_string())];
        let mut parser = Parser::new(&tokens);
        parser.parse_exp();
    }

    #[test]
    fn parse_exp_with_constant() {
        let tokens = vec![Token::Constant("100".to_string())];
        let mut parser = Parser::new(&tokens);
        let exp = parser.parse_exp();
        assert_eq!(exp.to_string(), "Constant(100)");
    }

    fn ident(value: &str) -> Token {
        Token::Identifier(value.to_string())
    }

    fn constant(value: &str) -> Token {
        Token::Constant(value.to_string())
    }

    fn lit(value: &str) -> Token {
        Token::Literal(value.to_string())
    }
}
