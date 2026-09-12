#pragma once
#include <vector>
#include <optional>
#include <string>
#include <variant>
#include <unordered_map>

struct TokenType
{
  enum class BinOperator
  {
    plus,
    minus,
    star,
    slash
  };

  enum class Assignment
  {
    assign
  };

  enum class Stmt
  {
    _return,
    pls_make,
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
    closed_paren
  };
};

using TokenKind = std::variant
<
  TokenType::BinOperator,
  TokenType::Assignment,
  TokenType::Expr,
  TokenType::Stmt,
  TokenType::Punctuation
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
  {"=>", TokenType::Assignment::assign},
  {";", TokenType::Punctuation::semi},
  {"(", TokenType::Punctuation::open_paren},
  {")", TokenType::Punctuation::closed_paren},
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

  struct Term
  {
    std::variant<TermIntLit *, TermIdent *> variant_term;
  };

  struct Expr;

  struct BinExpr
  {
    TokenType::BinOperator op;
    Expr *lhs;
    Expr *rhs;
  };

  struct Expr
  {
    std::variant<Term *, BinExpr *> variant_expr;
  };

  struct StmtPlsMake
  {
    Token ident;
    Expr *stmt_expr;
  };

  struct StmtExit
  {
    Expr *exit_expr;
  };

  struct Stmt
  {
    std::variant<StmtExit *, StmtPlsMake *> variant_stmt;
  };

  struct Prog
  {
    std::vector<Stmt> stmts;
  };
};