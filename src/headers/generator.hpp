#pragma once
#include <sstream>
#include <optional>
#include <unordered_map>

#include <./components.hpp>

class Generator
{
private:
  const std::optional<Node::Prog> prog;
  std::stringstream output;
  size_t stack_offset = 0;
  struct Var
  {
    size_t loc;
  };
  std::unordered_map<std::string, Var> vars = {};

  void push(std::string reg)
  {
    output << "   push " << reg << '\n';
    stack_offset++;
  }

  void pop(std::string reg)
  {
    output << "   pop " << reg << '\n';
    stack_offset--;
  }

public:
  inline explicit Generator(std::optional<Node::Prog> prog) : prog(prog) {};

  void gen_expr(Node::Expr expr)
  {
    struct ExprVisitor
    {
      Generator &gen;
      void operator()(Node::ExprIntLit &int_lit)
      {
        gen.output << "   mov rax, " << int_lit.intLit.value.value() << '\n';
        gen.push("rax");
      }
      void operator()(Node::ExprIdent &expr_ident)
      {

        if (gen.vars.find(expr_ident.ident.value.value()) == gen.vars.end())
        {
          std::cerr << "Undeclared variable: " << expr_ident.ident.value.value() << std::endl;
          exit(EXIT_FAILURE);
        }

        auto& var = gen.vars[expr_ident.ident.value.value()];
        gen.push("QWORD [rsp + " + std::to_string((gen.stack_offset - var.loc) * 8) + "]\n");
      }
    };

    ExprVisitor visitor{*this};
    std::visit(visitor, expr.var_expr);
  }

  void gen_stmt(Node::Stmt stmt)
  {
    struct StmtVisitor
    {
      Generator &gen;
      void operator()(Node::StmtExit &stmt_exit)
      {

        gen.gen_expr(stmt_exit.exit_expr);
        gen.output << "   mov rax, 60\n";
        gen.pop("rdi");
        gen.output << "   syscall\n";
      }
      void operator()(Node::StmtPlsMake &stmt_pls_make)
      {

        if (gen.vars.contains(stmt_pls_make.ident.value.value()))
        {
          std::cerr << "Identifier is already used!\n";
          exit(EXIT_FAILURE);
        }

        gen.gen_expr(stmt_pls_make.stmt_expr);
        gen.vars.insert({stmt_pls_make.ident.value.value(), Var{.loc = gen.stack_offset}});
      }
    };

    StmtVisitor visitor{*this};
    std::visit(visitor, stmt.var_stmt);
  }

  std::string generate()
  {
    output << "section .text\n";
    output << "   global main\n";
    output << "main:\n";

    for (Node::Stmt stmt : prog.value().stmts)
    {
      gen_stmt(stmt);
    }

    output << "   mov rax, 60\n";
    output << "   mov rdi, 0\n";
    output << "   syscall\n";

    return output.str();
  }
};