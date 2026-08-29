// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "../../../AST/AST.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <iostream>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{
/// @brief Lexer
class Lexer
{
  public:
	Lexer(std::string source);
	std::vector<Token> tokenize();

	struct Error
	{
		std::string message;
		size_t      line;
		size_t      column;
	};
	[[nodiscard]] std::optional<Error> getError() const
	{
		return lastError;
	}
	[[nodiscard]] const std::vector<Error> &getErrors() const
	{
		return errors;
	}

  private:
	std::string source;
	size_t      position = 0;
	size_t      line = 1;
	size_t      column = 1;

	std::optional<Error> lastError;
	std::vector<Error>   errors;

	void  reportError(const std::string &message, size_t errLine, size_t errColumn);
	char  peek();
	char  advance();
	bool  isAtEnd();
	void  skipWhitespace();
	void  skipShebang();
	Token scanToken();
	Token identifier();
	Token number();
	Token string();
	Token complexString();
};
} // namespace Phasor
