#pragma once
#include <vector>
#include <optional>
#include <string>
#include <variant>

enum class TokenType {
  _return,
  intLiteral,
  semi,
  open_paren,
  closed_paren,
  ident,
  pls_make,
  assignment
};

struct Token {
  TokenType type;
  std::optional<std::string> value;
};

namespace Node {
  struct ExprIntLit {
    Token intLit;
  };

  struct ExprIdent {
    Token ident;
  };

  struct Expr {
    std::variant<ExprIntLit, ExprIdent> var_expr;
  };

  struct StmtPlsMake {
    Token ident;
    Expr stmt_expr;
  };

  struct StmtExit {
    Expr exit_expr;
  };

  struct Stmt {
    std::variant<StmtExit, StmtPlsMake> var_stmt;
  };

  struct Prog {
    std::vector<Stmt> stmts;
  };
};