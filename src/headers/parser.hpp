#pragma once
#include <iostream>
#include <optional>
#include <vector>


// Header files
#include "./components.hpp"
#include "./ArenaAllocator.hpp"
#include "./helpers.hpp"

class Parser
{
private:
  size_t m_index;
  std::vector<Token> m_tokens;
  ArenaAllocator m_arena;

  // A function to look at a specific token
  std::optional<Token> peek(size_t offset = 0) const
  {
    if (m_index + offset >= m_tokens.size())
    {
      return std::nullopt;
    }
    else
    {
      return m_tokens.at(m_index + offset);
    }
  }

  // A function to consume the current token
  // and retrieve to the next token
  Token consume()
  {
    return m_tokens.at(m_index++);
  }

  template <typename T>
  std::optional<Token> tryConsume(std::string error_msg, T token_type, size_t index = 0)
  {
    if (peek(index).has_value() && isToken(peek(index).value().type, token_type))
    {
      return consume();
    }
    else
    {
      std::cerr << error_msg << '\n';
      return std::nullopt;
    }
  }

  template <typename T>
  std::optional<Token> tryConsume(T token_type, size_t index = 0)
  {
    if (peek(index).has_value() && isToken(peek(index).value().type, token_type))
    {
      return consume();
    }
    else
    {
      return std::nullopt;
    }
  }

  std::optional<Node::Term *> termParse()
  {
    Node::Term *term = m_arena.allocate<Node::Term>();

    if (auto val = tryConsume(TokenType::Expr::intLiteral))
    {
      Node::TermIntLit *int_lit_term = m_arena.allocate<Node::TermIntLit>();
      int_lit_term->intLit = val.value();

      term->variant_term = int_lit_term;

      return term;
    }

    else if (auto val = tryConsume(TokenType::Expr::ident))
    {
      Node::TermIdent *ident_term = m_arena.allocate<Node::TermIdent>();
      ident_term->ident = val.value();

      term->variant_term = ident_term;

      return term;
    }

    else
    {
      return std::nullopt;
    }
  }

  std::optional<Node::BinExpr *> binExprParse(Node::Expr *left_expr, u_short precedence = 1, std::string s = "left")
  {
    Node::Expr *lhs = left_expr;

    while (true)
    {
      auto op = peek();

      if (!op.has_value() || !isTokenType<TokenType::BinOperator>(op.value().type))
      {
        break;
      }

      auto bin_op = std::get<TokenType::BinOperator>(op.value().type);
      auto op_prec = binOpPrec(bin_op);

      if (!op_prec.has_value() || op_prec.value() < precedence)
      {
        break;
      }

      consume(); // Consume the operator token

      auto right = termParse();

      if (!right.has_value())
      {
        std::cerr << "ERROR: Expected an expression after the binary operator!\n";
        exit(EXIT_FAILURE);
      }

      Node::Expr *right_expr = m_arena.allocate<Node::Expr>();
      right_expr->variant_expr = right.value();

      u_short next_prec = op_prec.value() + 1;

      auto next = binExprParse(right_expr, next_prec);

      Node::Expr *rhs = right_expr;

      if (next.has_value())
      {
        rhs = m_arena.allocate<Node::Expr>();
        rhs->variant_expr = next.value();
      }

      Node::BinExpr *bin_expr = m_arena.allocate<Node::BinExpr>();

      bin_expr->op = bin_op;
      bin_expr->lhs = lhs;
      bin_expr->rhs = rhs;

      lhs = m_arena.allocate<Node::Expr>();
      lhs->variant_expr = bin_expr;
    }

    if (std::holds_alternative<Node::BinExpr *>(lhs->variant_expr))
    {
      return std::get<Node::BinExpr *>(lhs->variant_expr);
    }

    return std::nullopt;
  }

  std::optional<Node::Expr *> exprParse()
  {
    auto term = termParse();

    if (!term.has_value())
    {
      std::cerr << "ERROR: Expected an expression!\n";
      exit(EXIT_FAILURE);
    }

    Node::Expr *expr = m_arena.allocate<Node::Expr>();
    expr->variant_expr = term.value();

    if (peek().has_value() && isTokenType<TokenType::BinOperator>(peek().value().type))
    {
      auto bin_expr = binExprParse(expr);

      if (!bin_expr.has_value())
      {
        return std::nullopt;
      }

      expr = m_arena.allocate<Node::Expr>();
      expr->variant_expr = bin_expr.value();
    }

    return expr;
  }

  std::optional<Node::Stmt> stmtParse()
  {
    if (tryConsume(TokenType::Stmt::_return) && tryConsume(TokenType::Punctuation::open_paren))
    {
      Node::StmtExit *stmt_exit = m_arena.allocate<Node::StmtExit>();

      if (auto parsed_expr = exprParse())
      {
        stmt_exit->exit_expr = parsed_expr.value();
      }
      else
      {
        std::cerr << "Expression required!\n";
        exit(EXIT_FAILURE);
      }

      if (!tryConsume("Expected: ')'!", TokenType::Punctuation::closed_paren))
      {
        exit(EXIT_FAILURE);
      };

      if (!tryConsume("Expected: ';'!", TokenType::Punctuation::semi))
      {
        exit(EXIT_FAILURE);
      }

      return Node::Stmt{.variant_stmt = stmt_exit};
    }

    else if (tryConsume(TokenType::Stmt::pls_make))
    {

      if (auto ident = tryConsume("ERROR: Expecter an identifier!", TokenType::Expr::ident))
      {
        Node::StmtPlsMake *make_stmt = m_arena.allocate<Node::StmtPlsMake>();
        make_stmt->ident = ident.value();

        if (tryConsume("ERROR: Expected the assignment operator!", TokenType::Assignment::assign))
        {

          if (auto expr = exprParse())
          {
            make_stmt->stmt_expr = expr.value();
          }
          else
          {
            std::cerr << "Invalid expression! | pls_make <ident> = <expr>\n";
            exit(EXIT_FAILURE);
          }

          if (!tryConsume("Expected: ';'!", TokenType::Punctuation::semi))
          {
            exit(EXIT_FAILURE);
          }

          return Node::Stmt{.variant_stmt = make_stmt};
        }
        else
        {
          exit(EXIT_FAILURE);
        }
      }
      else
      {
        exit(EXIT_FAILURE);
      }
    }
    else
    {
      return std::nullopt;
    }
  }

public:
  inline explicit Parser(std::vector<Token> tokens)
      : m_tokens(tokens), m_arena(MEGA * 5), m_index(0)
  {
  }

  std::optional<Node::Prog> parse()
  {
    Node::Prog program_stmts;
    while (peek().has_value())
    {
      if (auto stmt = stmtParse())
      {
        program_stmts.stmts.push_back(stmt.value());
      }
      else
      {
        std::cerr << "Invalid statement!\n";
        exit(EXIT_FAILURE);
      }
    }
    return program_stmts;
  }
};