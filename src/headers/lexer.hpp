#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <vector>

// Header files
#include "./components.hpp"

class Lexer
{
private:
  std::string src;
  size_t index;

  std::optional<char> peek(size_t offset = 0) const
  {
    if (index + offset >= src.length())
    {
      return std::nullopt;
    }
    else
    {
      return src.at(index + offset);
    }
  }

  // A function to consume the current character
  // and get into the next character
  char consume()
  {
    return src.at(index++);
  }

public:
  inline explicit Lexer(std::string src) : src(src) { index = 0; }

  std::vector<Token> lex()
  {
    std::vector<Token> tokens;
    std::string buff;

    while (peek().has_value())
    {
      if (std::isalpha(peek().value()))
      {
        while (peek().has_value() && (std::isalnum(peek().value()) || peek().value() == '_'))
        {
          buff.push_back(consume());
        }
        if (buff == "gimme")
        {
          tokens.push_back({.type = TokenType::_return});
          buff.clear();
        }
        else if (buff == "pls_make") {
          tokens.push_back({.type = TokenType::pls_make});
          buff.clear();
        }
        else
        {
          tokens.push_back({.type = TokenType::ident, .value = buff});
          buff.clear();
          continue;
        }
      }
      else if (std::isdigit(peek().value()))
      {
        while (peek().has_value() && std::isdigit(peek().value()))
        {
          buff.push_back(consume());
        }
        tokens.push_back({.type = TokenType::intLiteral, .value = buff});
        buff.clear();
      }
      else if (std::isspace(peek().value()))
      {
        consume();
        continue;
      }
      else if (peek().value() == '=' && peek(1).value() == '>') {
        tokens.push_back({.type = TokenType::assignment});
        consume();
        consume();
      }
      else if (peek().value() == ';')
      {
        tokens.push_back({.type = TokenType::semi});
        consume();
      }
      else if (peek().value() == '(') {
        tokens.push_back({.type = TokenType::open_paren});
        consume();
      }
      else if (peek().value() == ')') {
        tokens.push_back({.type = TokenType::closed_paren});
        consume();
      }
      else
      {
        std::cerr << "Invalid character: " << peek().value() << std::endl;
        exit(EXIT_FAILURE);
      }
    }

    return tokens;
  }
};