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
      exit(EXIT_FAILURE);
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

    else if (auto val = tryConsume(TokenType::Punctuation::open_paren))
    {
      if (auto end = tryConsume(TokenType::Punctuation::closed_paren))
      {
        std::cerr << "ERROR: Expected an expression inside the parentheses '()'!\n";
        exit(EXIT_FAILURE);
      }

      auto first_term = termParse();

      Node::Term *term = m_arena.allocate<Node::Term>();
      term->variant_term = first_term.value()->variant_term;

      if (peek().has_value() && isTokenType<TokenType::BinOperator>(peek().value().type))
      {

        Node::Expr *expr = m_arena.allocate<Node::Expr>();
        expr->variant_expr = first_term.value();

        auto bin_expr = binExprParse(expr);

        if (!bin_expr.has_value())
        {
          return std::nullopt;
        }

        term = m_arena.allocate<Node::Term>();
        term->variant_term = bin_expr.value();
      }

      if (auto end = tryConsume("ERROR: Expected token : ')' to end the expression!", TokenType::Punctuation::closed_paren))
      {
        return term;
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

  std::optional<Node::AssignExpr* > assignExprParse()
  {
    auto ident = consume();
    Node::AssignExpr *assign_expr = m_arena.allocate<Node::AssignExpr>();
    assign_expr->ident = ident;

    consume();

    if (auto expr = exprParse())
    {
      assign_expr->stmt_expr = expr.value();
    }
    else
    {
      std::cerr << "Invalid expression! | pls_make <ident> = <expr>\n";
      exit(EXIT_FAILURE);
    }

    return assign_expr;
  }

  std::optional<Node::UnaryExpr *> unaryExprParse()
  {
    Node::UnaryExpr *u_expr = m_arena.allocate<Node::UnaryExpr>();
    
    auto ident = consume();
    u_expr->ident = ident;

    auto op = consume().type;
    auto u_op = std::get<TokenType::UnaryOperator>(op);
    u_expr->op = u_op;

    return u_expr;
  }

  std::optional<Node::Expr *> exprParse()
  {
    if (isToken(peek().value().type, TokenType::Expr::ident) && isToken(peek(1).value().type, TokenType::Assignment::assign))
    {
      Node::Expr *expr = m_arena.allocate<Node::Expr>();
      auto assign_expr = assignExprParse();
      expr->variant_expr = assign_expr.value();
      return expr;
    }

    if (isToken(peek().value().type, TokenType::Expr::ident) && isTokenType<TokenType::UnaryOperator>(peek(1).value().type))
    {
      Node::Expr *expr = m_arena.allocate<Node::Expr>();
      auto u_expr = unaryExprParse();
      expr->variant_expr = u_expr.value();
      return expr;
    }

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

    if (peek().has_value() && isTokenType<TokenType::CompOperator>(peek().value().type))
    {
      Node::CompExpr *comp_expr = m_arena.allocate<Node::CompExpr>();
      comp_expr->lhs = expr;

      auto comp_op = std::get<TokenType::CompOperator>(peek().value().type);
      comp_expr->op = comp_op;
      consume();

      auto r_term = termParse();

      if (!r_term.has_value())
      {
        std::cerr << "ERROR: Expected an expression!\n";
        exit(EXIT_FAILURE);
      }

      Node::Expr *r_expr = m_arena.allocate<Node::Expr>();
      r_expr->variant_expr = r_term.value();

      if (peek().has_value() && isTokenType<TokenType::BinOperator>(peek().value().type))
      {
        auto r_bin_expr = binExprParse(r_expr);

        if (!r_bin_expr.has_value())
        {
          return std::nullopt;
        }

        r_expr = m_arena.allocate<Node::Expr>();
        r_expr->variant_expr = r_bin_expr.value();
      }

      comp_expr->rhs = r_expr;

      expr = m_arena.allocate<Node::Expr>();

      expr->variant_expr = comp_expr;
    }

    return expr;
  }

  std::optional<Node::Scope *> scopeParse()
  {
    Node::Scope *scope = m_arena.allocate<Node::Scope>();
    while (peek().has_value())
    {
      if (isTokenType<TokenType::Stmt>(peek().value().type) || isToken(peek().value().type, TokenType::Expr::ident) || isToken(peek().value().type, TokenType::Punctuation::open_curly))
      {
        auto stmt = stmtParse();
        scope->stmts.push_back(stmt.value());
      }
      else if (isToken(peek().value().type, TokenType::Punctuation::closed_curly))
      {
        consume();
        break;
      }
      else
      {
        std::cerr << "Expected: '}'\n";
        exit(EXIT_FAILURE);
      }
    }
    if (m_index >= m_tokens.size())
    {
      std::cerr << "Expected: '}'\n";
      exit(EXIT_FAILURE);
    }
    return scope;
  }

  Node::StmtCheck *stmtCheckHandle(bool otherwise_check = false)
  {
    if (otherwise_check)
    {
      consume();
    }
    consume();
    tryConsume("Expected: '('!", TokenType::Punctuation::open_paren);

    Node::StmtCheck *check_stmt = m_arena.allocate<Node::StmtCheck>();
    Node::Expr *check_expr = m_arena.allocate<Node::Expr>();

    if (auto expr = exprParse())
    {
      check_expr = expr.value();
    }
    else
    {
      std::cerr << "ERROR: Invalid expression!\n";
      exit(EXIT_FAILURE);
    }

    check_stmt->expr = check_expr;

    tryConsume("Expected: ')'!", TokenType::Punctuation::closed_paren);
    tryConsume("ERROR: Expected a scope for the check statement!", TokenType::Punctuation::open_curly);
    
    Node::Scope *check_scope = m_arena.allocate<Node::Scope>();
    if (auto scope = scopeParse())
    {
      check_scope = scope.value();
    }
    else
    {
      std::cerr << "ERROR: Invalid scope!\n";
      exit(EXIT_FAILURE);
    }

    check_stmt->scope = check_scope;
    return check_stmt;
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

        if (tryConsume("Expected: '=>'!", TokenType::Assignment::assign))
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
    else if (isToken(peek().value().type, TokenType::Expr::ident))
    {
      Node::Stmt *stmt = m_arena.allocate<Node::Stmt>();

      if (isTokenType<TokenType::UnaryOperator>(peek(1).value().type))
      {
        Node::StmtUnary *u_stmt = m_arena.allocate<Node::StmtUnary>();

        auto expr = exprParse();
        u_stmt->u_expr = expr.value();
        stmt->variant_stmt = u_stmt;
      }
      else
      {
        Node::StmtAssign *assign_stmt = m_arena.allocate<Node::StmtAssign>();
  
        auto expr = exprParse();
        assign_stmt->assign_expr = expr.value();
        stmt->variant_stmt = assign_stmt;
      }

      tryConsume("Expected: ';'!", TokenType::Punctuation::semi);
      return *stmt;
    }
    else if (tryConsume(TokenType::Punctuation::open_curly))
    {
      Node::Stmt *stmt = m_arena.allocate<Node::Stmt>();
      if (auto scope = scopeParse())
      {
        stmt->variant_stmt = scope.value();
      }
      else
      {
        std::cerr << "ERROR: Invalid scope!\n";
        exit(EXIT_FAILURE);
      }
      return *stmt;
    }
    else if (isToken(peek().value().type, TokenType::Stmt::check))
    {
      Node::StmtControlFlow *control_flow = m_arena.allocate<Node::StmtControlFlow>();

      Node::StmtCheck *check_stmt = m_arena.allocate<Node::StmtCheck>();

      check_stmt = stmtCheckHandle();
      control_flow->check_stmt = check_stmt;

      std::vector<Node::StmtCheck *> checks_vec;

      while (true)
      {
        if (isToken(peek().value().type, TokenType::Stmt::otherwise) && isToken(peek(1).value().type, TokenType::Stmt::check))
        {
          Node::StmtCheck *otherwise_check_stmt = m_arena.allocate<Node::StmtCheck>();
          otherwise_check_stmt = stmtCheckHandle(true);
          checks_vec.push_back(otherwise_check_stmt);
        }
        else
        {
          break;
        }
      }

      control_flow->checks_vec = checks_vec;

      if (tryConsume(TokenType::Stmt::otherwise))
      {
        Node::StmtOtherwise *otherwise_stmt = m_arena.allocate<Node::StmtOtherwise>();
        tryConsume("ERROR: Expected a scope for the check statement!", TokenType::Punctuation::open_curly);
        
        Node::Scope *otherwise_scope = m_arena.allocate<Node::Scope>();
        if (auto scope = scopeParse())
        {
          otherwise_scope = scope.value();
        }
        else
        {
          std::cerr << "ERROR: Invalid scope!\n";
          exit(EXIT_FAILURE);
        }

        otherwise_stmt->scope = otherwise_scope;
        control_flow->otherwise_stmt = otherwise_stmt;
      }

      return Node::Stmt{.variant_stmt = control_flow};
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