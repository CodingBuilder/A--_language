#pragma once
#include <sstream>
#include <optional>
#include <unordered_map>
#include <cassert>
#include <stack>

#include "./components.hpp"
#include "./helpers.hpp"

class Generator
{
private:
  struct Var
  {
    size_t offset;
  };

  struct StackFrame
  {
    size_t next_offset;
    StackFrame *parent;
    std::unordered_map<std::string, Var> vars;
  };

  const std::optional<Node::Prog> m_prog;
  std::stringstream m_output;
  size_t m_stack_offset;
  StackFrame m_global_frame{.next_offset = 8, .parent = nullptr, .vars = {}};
  StackFrame *m_current_frame;
  size_t m_label_count;

  std::string createLabel(std::string label = "label")
  {
    return label + std::to_string(m_label_count++);
  }

  void mov(std::string dest, std::string src)
  {
    m_output << "\tmov " << dest << ", " << src << '\n';
  }

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

  StackFrame *findVarFrame(const std::string &ident)
  {
    for (StackFrame *frame = m_current_frame; frame != nullptr; frame = frame->parent)
    {
      if (frame->vars.contains(ident))
      {
        return frame;
      }
    }
    return nullptr;
  }

  void varAddress(std::string ident, StackFrame *target_frame)
  {
    mov("r11", "rbp");

    StackFrame *frame = m_current_frame;
    while (frame != target_frame)
    {
      if (frame == nullptr)
      {
        std::cerr << "ERROR: Targeted frame not found!\n";
        exit(EXIT_FAILURE);
      }

      mov("r11", "QWORD [r11]");
      frame = frame->parent;
    }

    size_t offset = target_frame->vars[ident].offset;
    m_output << "\tlea r11, [r11 - " << offset << "]\n";
  }

public:
  inline explicit Generator(std::optional<Node::Prog> prog)
      : m_prog(prog),
        m_stack_offset(0),
        m_label_count(0),
        m_current_frame(&m_global_frame)
        {
          m_current_frame->parent = nullptr;
          m_current_frame->next_offset = 8;
        };

  void genScope(Node::Scope *scope)
  {
    StackFrame *parent_frame = m_current_frame;
    StackFrame frame{.next_offset = 8, .parent = parent_frame, .vars = {}};
    m_current_frame = &frame;

    m_output << "\tpush rbp\n";
    m_output << "\tmov rbp, rsp\n";

    for (auto stmt : scope->stmts)
    {
      genStmt(stmt);
    }

    m_output << "\tmov rsp, rbp\n";
    m_output << "\tpop rbp\n";

    m_current_frame = m_current_frame->parent;
  }

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
        auto ident = ident_term->ident.value.value();
        StackFrame *target_frame = gen.findVarFrame(ident);

        if (target_frame == nullptr)
        {
          std::cerr << "Undeclared variable: " << ident_term->ident.value.value() << '\n';
          exit(EXIT_FAILURE);
        }

        gen.varAddress(ident, target_frame);
        gen.push("QWORD [r11]");
      }

      void operator()(Node::BinExpr *bin_expr_term)
      {
        gen.genBinExpr(bin_expr_term);
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

  void genCompExpr(Node::CompExpr *comp_expr)
  {
    Generator &gen = *this;

    gen.genExpr(comp_expr->lhs);
    gen.genExpr(comp_expr->rhs);

    gen.pop("rbx");
    gen.pop("rax");

    gen.m_output << "\tcmp rax, rbx\n";
    std::string false_comparsion = gen.createLabel("false_comparsion");
    std::string complete = gen.createLabel("complete");

    if (isToken(comp_expr->op, TokenType::CompOperator::greater))
    {
      gen.m_output << "\tjle " << false_comparsion << '\n';
    }
    else if (isToken(comp_expr->op, TokenType::CompOperator::less))
    {
      gen.m_output << "\tjge " << false_comparsion << '\n';
    }
    else if (isToken(comp_expr->op, TokenType::CompOperator::equal))
    {
      gen.m_output << "\tjne " << false_comparsion << '\n';
    }

    gen.m_output << "\tmov rax, 1\n";
    gen.m_output << "\tjmp " << complete << '\n';

    gen.m_output << false_comparsion << ":\n";
    gen.m_output << "\tmov rax, 0\n";

    gen.m_output << complete << ":\n";
    gen.push("rax");
  }

  void genAssignExpr(Node::AssignExpr *assign_expr)
  {
    Generator &gen = *this;
    auto ident = assign_expr->ident.value.value();
    StackFrame *target_frame = gen.findVarFrame(ident);

    if (target_frame == nullptr)
    {
      std::cerr << "Undeclared variable: " << ident << '\n';
      exit(EXIT_FAILURE);
    }

    gen.genExpr(assign_expr->stmt_expr);
    gen.pop("rax");

    gen.varAddress(ident, target_frame);
    gen.mov("QWORD [r11]", "rax");
  }

  void genUnaryExpr(Node::UnaryExpr *u_expr)
  {
    Generator &gen = *this;
    auto ident = u_expr->ident.value.value();
    StackFrame *target_frame = gen.findVarFrame(ident);

    if (target_frame == nullptr)
    {
      std::cerr << "Undeclared variable: " << ident << '\n';
      exit(EXIT_FAILURE);
    }

    gen.varAddress(ident, target_frame);

    if (u_expr->op == TokenType::UnaryOperator::inc)
    {
      gen.m_output << "\tinc QWORD [r11]\n";
    }
    else
    {
      gen.m_output << "\tdec QWORD [r11]\n";
    }
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
      void operator()(Node::CompExpr *comp_expr)
      {
        gen.genCompExpr(comp_expr);
      }
      void operator()(Node::AssignExpr *assign_expr)
      {
        gen.genAssignExpr(assign_expr);
      }
      void operator()(Node::UnaryExpr *u_expr)
      {
        gen.genUnaryExpr(u_expr);
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
        auto ident = stmt_pls_make->ident.value.value();

        if (gen.m_current_frame->vars.contains(ident))
        {
          std::cerr << "ERROR: Identifier << " << ident << " >> is already used!\n";
          exit(EXIT_FAILURE);
        }

        gen.genExpr(stmt_pls_make->stmt_expr);
        gen.pop("rax");

        size_t offset = gen.m_current_frame->next_offset;
        gen.m_current_frame->next_offset += 8;

        gen.m_output << "\tsub rsp, 8\n";

        std::string dest = "QWORD [rbp - " + std::to_string(offset) + "]";
        gen.mov(dest, "rax");

        gen.m_current_frame->vars.insert({ident, Generator::Var{.offset = offset}});
      }
      void operator()(Node::Scope *stmt_scope)
      {
        gen.genScope(stmt_scope);
      }
      void operator()(Node::StmtControlFlow *stmt_control_flow)
      {
        std::string complete_label = gen.createLabel("complete_label");

        auto check_stmt = stmt_control_flow->check_stmt;

        gen.genExpr(check_stmt->expr);
        gen.pop("rax");
        gen.m_output << "\tcmp rax, 0\n";

        std::string next_label = gen.createLabel("next_label");
        gen.m_output << "\tje " << next_label << '\n';

        gen.genScope(check_stmt->scope);
        gen.m_output << "\tjmp " << complete_label << '\n';

        gen.m_output << next_label << ":\n";

        for (auto &check : stmt_control_flow->checks_vec)
        {
          next_label = gen.createLabel("next_label");

          gen.genExpr(check->expr);
          gen.pop("rax");
          gen.m_output << "\tcmp rax, 0\n";
          gen.m_output << "\tje " << next_label << '\n';

          gen.genScope(check->scope);
          gen.m_output << "\tjmp " << complete_label << '\n';

          gen.m_output << next_label << ":\n";
        }

        if (stmt_control_flow->otherwise_stmt.has_value())
        {
          gen.genScope(stmt_control_flow->otherwise_stmt.value()->scope);
        }

        gen.m_output << complete_label << ":\n";
      }
      void operator()(Node::StmtAssign *assign_stmt)
      {
        gen.genExpr(assign_stmt->assign_expr);
      }
      void operator()(Node::StmtUnary *u_stmt)
      {
        gen.genExpr(u_stmt->u_expr);
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

    push("rbp");
    mov("rbp", "rsp");

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