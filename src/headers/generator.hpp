#pragma once
#include <sstream>
#include <optional>
#include <unordered_map>
#include <cassert>

#include "./components.hpp"

class Generator
{
private:
  struct Var
  {
    size_t loc;
  };

  const std::optional<Node::Prog> m_prog;
  std::stringstream m_output;
  size_t m_stack_offset;
  std::unordered_map<std::string, Var> vars = {};

  void push(std::string reg)
  {
    m_output << "\tpush " << reg << '\n';
    m_stack_offset++;
  }

  void pop(std::string reg)
  {
    m_output << "\tpop " << reg << '\n';
    m_stack_offset--;
  }

public:
  inline explicit Generator(std::optional<Node::Prog> prog)
      : m_prog(prog),
        m_stack_offset(0) {};

  void genTerm(Node::Term *term)
  {
    struct TermVisitor
    {
      Generator &gen;

      void operator()(Node::TermIntLit *int_lit_term)
      {
        gen.m_output << "\tmov rax, " << int_lit_term->intLit.value.value() << '\n';
        gen.push("rax");
      }

      void operator()(Node::TermIdent *ident_term)
      {

        if (gen.vars.find(ident_term->ident.value.value()) == gen.vars.end())
        {
          std::cerr << "Undeclared variable: " << ident_term->ident.value.value() << std::endl;
          exit(EXIT_FAILURE);
        }

        auto &var = gen.vars[ident_term->ident.value.value()];
        auto offset = (gen.m_stack_offset - var.loc) * 8;
        gen.push("QWORD [rsp + " + std::to_string(offset) + "]");
      }
    };

    TermVisitor visitor{*this};
    std::visit(visitor, term->variant_term);
  }

  void genBinExpr(Node::BinExpr *bin_expr)
  {
    Generator &gen = *this;

    gen.genExpr(bin_expr->lhs);
    gen.genExpr(bin_expr->rhs);

    gen.pop("rbx");
    gen.pop("rax");

    if (bin_expr->op == TokenType::BinOperator::plus)
    {
      gen.m_output << "\tadd rax, rbx\n";
    }

    else if (bin_expr->op == TokenType::BinOperator::minus)
    {
      gen.m_output << "\tsub rax, rbx\n";
    }

    else if (bin_expr->op == TokenType::BinOperator::star)
    {
      gen.m_output << "\timul rbx\n";
    }

    else if (bin_expr->op == TokenType::BinOperator::slash)
    {
      gen.m_output << "\tcmp rbx, 0\n";
      gen.m_output << "\tje division_by_zero\n";

      gen.m_output << "\txor rdx, rdx\n";

      gen.m_output << "\tidiv rbx\n";
    }

    gen.push("rax");
  }

  void genExpr(Node::Expr *expr)
  {
    struct ExprVisitor
    {
      Generator &gen;

      void operator()(Node::Term *term)
      {
        gen.genTerm(term);
      }

      void operator()(Node::BinExpr *bin_expr)
      {
        gen.genBinExpr(bin_expr);
      }
    };

    ExprVisitor visitor{*this};
    std::visit(visitor, expr->variant_expr);
  }

  void genStmt(Node::Stmt stmt)
  {
    struct StmtVisitor
    {
      Generator &gen;
      void operator()(Node::StmtExit *stmt_exit)
      {

        gen.genExpr(stmt_exit->exit_expr);
        gen.m_output << "\tmov rax, 60\n";
        gen.pop("rdi");
        gen.m_output << "\tsyscall\n";
      }
      void operator()(Node::StmtPlsMake *stmt_pls_make)
      {

        if (gen.vars.contains(stmt_pls_make->ident.value.value()))
        {
          std::cerr << "ERROR: Identifier << " << stmt_pls_make->ident.value.value() << " >> is already used!\n";
          exit(EXIT_FAILURE);
        }

        gen.genExpr(stmt_pls_make->stmt_expr);
        gen.vars.insert({stmt_pls_make->ident.value.value(), Var{.loc = gen.m_stack_offset}});
      }
    };

    StmtVisitor visitor{*this};
    std::visit(visitor, stmt.variant_stmt);
  }

  std::string generate()
  {
    m_output << "section .text\n";
    m_output << "\tglobal main\n";
    m_output << "\nmain:\n";

    for (auto stmt : m_prog.value().stmts)
    {
      genStmt(stmt);
    }

    m_output << "\tmov rax, 60\n";
    m_output << "\tmov rdi, 0\n";
    m_output << "\tsyscall\n";

    m_output << "division_by_zero:\n";
    m_output << "\tmov rax, 60\n";
    m_output << "\tmov rdi, 1\n";
    m_output << "\tsyscall\n";

    return m_output.str();
  }
};