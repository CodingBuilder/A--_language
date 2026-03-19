#pragma once
#include <iostream>
#include <optional>
#include <vector>

// Header files
#include "./components.hpp"

class Parser
{
private:
  size_t index;
  std::vector<Token> tokens;

  std::optional<Token> peek(size_t offset = 0) const
  {
    if (index + offset >= tokens.size())
    {
      return std::nullopt;
    }
    else
    {
      return tokens.at(index + offset);
    }
  }

  // A function to consume the current token
  // and retrieve to the next token
  Token consume()
  {
    return tokens.at(index++);
  }

  std::optional<Node::Expr> exprParse()
  {
    if (peek().has_value() && peek().value().type == TokenType::intLiteral)
    {
      return Node::Expr{.var_expr = Node::ExprIntLit {.intLit = consume()}};
    }
    else if (peek().has_value() && peek().value().type == TokenType::ident) {
      return Node::Expr{.var_expr = Node::ExprIdent{.ident = consume()}};
    }
    else
    {
      return std::nullopt;
    }
  }

  std::optional<Node::Stmt> stmtParse() {
    if (peek().value().type == TokenType::_return && peek(1).has_value() && peek(1).value().type == TokenType::open_paren)
    {
      Node::StmtExit stmt_exit;
      consume();
      consume();

      if (auto parsed_expr = exprParse())
      {
        stmt_exit = {.exit_expr = parsed_expr.value()};
      }
      else
      {
        std::cerr << "Expression required!\n";
        exit(EXIT_FAILURE);
      }

      if (peek().has_value() && peek().value().type == TokenType::closed_paren) {
        consume();
      }
      else {
        std::cerr << "Expected: ')'!\n";
        exit(EXIT_FAILURE);
      }

      if (peek().has_value() && peek().value().type == TokenType::semi)
      {
        consume();
      }
      else
      {
        std::cerr << "Expected: ';'!\n";
        exit(EXIT_FAILURE);
      }
      
      return Node::Stmt{.var_stmt = stmt_exit};
    }

    else if (peek().value().type == TokenType::pls_make && peek(1).has_value() && peek(1).value().type == TokenType::ident && peek(2).value().type == TokenType::assignment)
    {
      consume();
      Node::StmtPlsMake make_stmt = {.ident = consume()};
      consume();
      if (auto expr = exprParse()) {
        make_stmt.stmt_expr = expr.value();
      }
      else {
        std::cerr << "Invalid expression! | pls_make <ident> = <expr>\n";
        exit(EXIT_FAILURE);
      }

      if (peek().has_value() && peek().value().type == TokenType::semi) {
        consume();
      }
      else {
        std::cerr << "Expected: ';'!\n";
        exit(EXIT_FAILURE);
      }

      return Node::Stmt{.var_stmt = make_stmt};
    }
    else
    {
      return std::nullopt;
    }
  }

public:
  inline explicit Parser(std::vector<Token> tokens) : tokens(tokens) { index = 0; }

  std::optional<Node::Prog> parse()
  {
    Node::Prog program_stmts;
    while (peek().has_value())
    {
      if (auto stmt = stmtParse()) {
        program_stmts.stmts.push_back(stmt.value());
      }
      else {
        std::cerr << "Invalid statement!\n";
        exit(EXIT_FAILURE);
      }
    }
    return program_stmts;
  }
};