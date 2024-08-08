#include <algorithm>
#include <memory>
#include <vector>

namespace Tacky {
class Val {
  public:
    virtual ~Val() = default;
    virtual std::string const value() = 0;
};

class Constant : public Val {
    std::string _int{};

  public:
    Constant(std::string i) : _int(i) {}
    std::string const value() override { return _int; }
};

class Var : public Val {
    std::string _identifier{};

  public:
    Var(std::string i) : _identifier(i) {}
    std::string const value() override { return _identifier; }
};

class UnaryOperator {
  public:
    virtual ~UnaryOperator() = default;
};

class Complement : public UnaryOperator {};
class Negate : public UnaryOperator {};

class Instruction {
  public:
    virtual ~Instruction() = default;
};

class Return : public Instruction {
    std::unique_ptr<Val> _val{};
};

class Unary : public Instruction {
    std::unique_ptr<UnaryOperator> _op{};
    std::unique_ptr<Val> _src{};
    std::unique_ptr<Val> _dst{};

  public:
    Unary(std::unique_ptr<UnaryOperator> op, std::unique_ptr<Val> src,
          std::unique_ptr<Val> dst)
        : _op(std::move(op)), _src(std::move(src)), _dst(std::move(dst)) {}
};

class Function {
    std::string _identifier{};
    std::vector<Instruction> _body{};
};

class Program {
    std::unique_ptr<Function> _fn{};
};
} // namespace Tacky
