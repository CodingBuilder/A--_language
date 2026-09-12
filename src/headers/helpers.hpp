#pragma once
#include <string>
#include <functional>
#include <optional>

#include "components.hpp"

std::string trim(const std::string &str, std::string chars_to_trim)
{
  size_t start = str.find_first_not_of(chars_to_trim);

  if (start == std::string::npos)
    return "";

  size_t end = str.find_last_not_of(chars_to_trim);
  return str.substr(start, end - start + 1);
}

template <typename T>
bool isToken(TokenKind const& token, T token_type)
{
  return std::holds_alternative<T>(token) && std::get<T>(token) == token_type;
}

template <typename T>
bool isTokenType(TokenKind const& child_token)
{
  return std::holds_alternative<T>(child_token);
}

std::optional<u_short> binOpPrec(TokenType::BinOperator bin_op_token)
{
  switch(bin_op_token)
  {
    case TokenType::BinOperator::plus:
    case TokenType::BinOperator::minus:
      return 1;

    case TokenType::BinOperator::star:
    case TokenType::BinOperator::slash:
      return 2;

    default:
      return std::nullopt;
  }
}