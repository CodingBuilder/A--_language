#pragma once
#include <string>

std::string trim(const std::string& str, std::string chars_to_trim) {
  size_t start = str.find_first_not_of(chars_to_trim);

  if (start == std::string::npos) return "";

  size_t end = str.find_last_not_of(chars_to_trim);
  return str.substr(start, end - start + 1);
}