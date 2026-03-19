# 🚀 AMM Compiler

**AMM** is a custom low-level programming language and compiler built from scratch. This project focuses on understanding the internals of compiler construction, memory management, and x86_64 assembly generation for Linux.

---

## 🛠 Current Progress (Stage 1)
The compiler currently handles the core infrastructure and basic program flow:

* **Lexical Analysis:** Custom hand-written lexer to tokenize AMM syntax.
* **AST Generation:** Building an Abstract Syntax Tree to represent program logic.
* **Variable Declaration:** Support for integer variables (`var`) and memory allocation.
* **System Calls:** Implementation of the `exit` statement, mapped directly to Linux **syscall 60**.
* **Backend:** Generates valid **x86_64 Assembly** compatible with Linux environments (WSL2/Ubuntu).

---

## 💻 Code Example
Here is what a valid `.amm` file looks like right now:

```amm
pls_make x => 10;
pls_make y => x;
gimme(y);