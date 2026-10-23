#pragma once
#include <iostream>
#include <optional>
#include <string>
#include <vector>

// Header files
#include "./components.hpp"
#include "./helpers.hpp"

class Lexer
{
private:
  std::string m_src;
  size_t m_index;

  // A function to look at a specific character
  std::optional<char> peek(size_t offset = 0) const
  {
    if (m_index + offset >= m_src.length())
    {
      return std::nullopt;
    }
    else
    {
      return m_src.at(m_index + offset);
    }
  }

  // A function to consume the current character
  // and get into the next character
  char consume()
  {
    return m_src.at(m_index++);
  }

  bool isPrefix(std::string pre)
  {
    for (auto const& [k, v] : lexer_map)
    {
      if (k.length() > pre.length() && k.starts_with(pre))
      {
        return true;
      }
    }

    return false;
  }

public:
  inline explicit Lexer(std::string src)
      : m_src(src), m_index(0)
  {
  }

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

        auto tok = lexer_map.find(buff);

        if (tok != lexer_map.end())
        {
          tokens.push_back({.type = tok->second});
          buff.clear();
        }
        else
        {
          tokens.push_back({.type = TokenType::Expr::ident, .value = buff});
          buff.clear();
        }
      }

      else if (std::isdigit(peek().value()))
      {
        while (peek().has_value() && std::isdigit(peek().value()))
        {
          buff.push_back(consume());
        }

        tokens.push_back({.type = TokenType::Expr::intLiteral, .value = buff});
        buff.clear();
      }

      else if (std::isspace(peek().value()))
      {
        consume();
      }

      else if (peek().value() == '(' && peek(1).has_value() && peek(1).value() == ':')
      {
        consume();
        consume();
        
        while (true)
        {
          if (peek().has_value() && peek().value() == ':' && peek(1).has_value() && peek(1).value() == ')')
          {
            consume();
            consume();
            break;
          }
          consume();
        }
      }

      else
      {   
        size_t last_valid_index = m_index;
        std::optional<TokenKind> last_token;

        while ((peek().has_value() && !std::isspace(peek().value()) && !std::isalnum(peek().value())))
        {
          buff.push_back(consume());

          auto tok = lexer_map.find(buff);

          if (tok != lexer_map.end())
          {
            last_token = tok->second;
            last_valid_index = m_index;
          }

          if (!isPrefix(buff))
          {
            break;
          }
        }

        if (last_token.has_value())
        {
          m_index = last_valid_index;
          tokens.push_back({.type = last_token.value()});
          buff.clear();
        }
        else
        {
          std::cerr << "ERROR: Invalid token: " << buff << '\n';
          exit(EXIT_FAILURE);
        }
      }
    }

    return tokens;
  }
};