// AMM\A-- (Ahmed minus minus) language project
// by me (CodingBuilder) :)

#include <iostream>
#include <fstream>
#include <sstream>
#include <optional>
#include <vector>
#include <cctype>
#include <cstdlib>

// Header files
#include "./headers/components.hpp"
#include "./headers/lexer.hpp"
#include "./headers/parser.hpp"
#include "./headers/generator.hpp"
#include "./headers/helpers.hpp"

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    std::cerr << "Incorrect usage. You should type...\n";
    std::cerr << "AMM <input.amm>\n";
    return EXIT_FAILURE;
  }

  std::string file_name = trim(argv[1], " \t\n");

  size_t last_dot = file_name.find_last_of(".");

  if (last_dot == std::string::npos || last_dot == 0)
  {
    std::cerr << "Error: Invalid filename. filename is empty or missing a name before the extension.\n";
    exit(EXIT_FAILURE);
  }

  std::string extension = file_name.substr(last_dot);
  for (char &c : extension)
    tolower(c);

  if (extension != ".amm" && extension != ".a--")
  {
    std::cerr << "Error: Wrong type of files, filename formula should be: <filename>.amm or <filename>.a--\n";
    exit(EXIT_FAILURE);
  }

  std::string content;
  {
    std::stringstream contentStream;
    std::ifstream MY_AMM_FILE(file_name);
    contentStream << MY_AMM_FILE.rdbuf();
    content = contentStream.str();
  }

  Lexer lexer(content);
  std::vector<Token> fileTokens = lexer.lex();
  
  Parser parser(fileTokens);
  std::optional<Node::Prog> tree = parser.parse();
  
  if (!tree.has_value())
  {
    std::cerr << "No return statement found!\n";
    exit(EXIT_FAILURE);
  }

  Generator generator(tree);
  std::string asmCode = generator.generate();

  {
    std::ofstream MY_ASM_FILE("out.asm");
    MY_ASM_FILE << asmCode;
  }

  system("nasm -f elf64 ./out.asm -o out.o");
  system("gcc -no-pie out.o -o out -z noexecstack");

  return EXIT_SUCCESS;
}