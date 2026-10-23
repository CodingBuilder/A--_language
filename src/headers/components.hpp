#pragma once
#include <vector>
#include <optional>
#include <string>
#include <variant>
#include <unordered_map>

struct TokenType
{
  enum class UnaryOperator
  {
    inc,
    dec
  };

  enum class BinOperator
  {
    plus,
    minus,
    star,
    slash
  };

  enum class CompOperator
  {
    less,
    greater,
    equal
  };

  enum class Assignment
  {
    assign
  };

  enum class Stmt
  {
    _return,
    pls_make,
    check,
    otherwise,
    _while
  };

  enum class Expr
  {
    ident,
    intLiteral
  };

  enum class Punctuation
  {
    semi,
    open_paren,
    closed_paren,
    open_curly,
    closed_curly
  };
};

using TokenKind = std::variant
<
  TokenType::UnaryOperator,
  TokenType::BinOperator,
  TokenType::Assignment,
  TokenType::Expr,
  TokenType::Stmt,
  TokenType::Punctuation,
  TokenType::CompOperator
>;

struct Token
{
  TokenKind type;
  std::optional<std::string> value;
};

std::unordered_map<std::string, TokenKind> lexer_map = {
  {"gimme", TokenType::Stmt::_return},
  {"pls_make", TokenType::Stmt::pls_make},
  {"+", TokenType::BinOperator::plus},
  {"-", TokenType::BinOperator::minus},
  {"*", TokenType::BinOperator::star},
  {"/", TokenType::BinOperator::slash},
  {"++", TokenType::UnaryOperator::inc},
  {"--", TokenType::UnaryOperator::dec},
  {"=>", TokenType::Assignment::assign},
  {";", TokenType::Punctuation::semi},
  {"(", TokenType::Punctuation::open_paren},
  {")", TokenType::Punctuation::closed_paren},
  {"{", TokenType::Punctuation::open_curly},
  {"}", TokenType::Punctuation::closed_curly},
  {"<", TokenType::CompOperator::less},
  {">", TokenType::CompOperator::greater},
  {"<=>", TokenType::CompOperator::equal},
  {"less_than", TokenType::CompOperator::less},
  {"greater_than", TokenType::CompOperator::greater},
  {"equal", TokenType::CompOperator::equal},
  {"check", TokenType::Stmt::check},
  {"otherwise", TokenType::Stmt::otherwise},
};

namespace Node
{
  struct TermIntLit
  {
    Token intLit;
  };

  struct TermIdent
  {
    Token ident;
  };

  struct BinExpr;

  struct Term
  {
    std::variant<TermIntLit *, TermIdent *, BinExpr *> variant_term;
  };

  struct Expr;

  struct BinExpr
  {
    TokenType::BinOperator op;
    Expr *lhs;
    Expr *rhs;
  };
  
  struct CompExpr
  {
    TokenType::CompOperator op;
    Expr *lhs;
    Expr *rhs;
  };

  struct AssignExpr
  {
    Token ident;
    Expr* stmt_expr;
  };

  struct UnaryExpr
  {
    Token ident;
    TokenType::UnaryOperator op;
  };

  struct Expr
  {
    std::variant<Term *, BinExpr *, CompExpr *, AssignExpr *, UnaryExpr *> variant_expr;
  };
  
  struct StmtPlsMake
  {
    Token ident;
    Expr *stmt_expr;
  };
  
  struct StmtAssign
  {
    Expr* assign_expr;
  };

  struct StmtUnary
  {
    Expr* u_expr;
  };

  struct Stmt;

  struct Scope
  {
    std::vector<Stmt> stmts;
  };
  
  struct StmtCheck
  {
    Expr *expr;
    Scope *scope;
  };
  
  struct StmtOtherwise
  {
    Scope *scope;
  };

  struct StmtControlFlow
  {
    StmtCheck *check_stmt;
    std::vector<StmtCheck *> checks_vec;
    std::optional<StmtOtherwise *> otherwise_stmt;
  };

  struct StmtExit
  {
    Expr *exit_expr;
  };

  struct Stmt
  {
    std::variant<StmtExit *, StmtPlsMake *, StmtAssign*, StmtUnary *, Scope *, StmtControlFlow *> variant_stmt;
  };

  struct Prog
  {
    std::vector<Stmt> stmts;
  };
};