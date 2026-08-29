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
#include "../../AST/AST.hpp"
#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <filesystem>

namespace Phasor
{
class LSP
{
  public:
	struct Diagnostic
	{
		std::string message;
		size_t      startLine;
		size_t      startColumn;
		size_t      endLine;
		size_t      endColumn;
	};

	enum class SymbolKind
	{
		Function,
		ForwardDecl,
		Struct,
		Variable,
		Parameter,
		Field,
	};

	struct SymbolInfo
	{
		std::string name;
		SymbolKind  kind = SymbolKind::Variable;
		AST::Node  *declaration = nullptr;
		std::string signature;
		std::string typeName;
		std::string typeDisplay;
		size_t      declLine = 0;
		size_t      declColumn = 0;
		std::vector<std::pair<std::string, std::string>> params;
	};

	struct Scope
	{
		size_t                                      startLine = 0, startColumn = 0;
		size_t                                      endLine = SIZE_MAX, endColumn = SIZE_MAX;
		Scope                                       *parent = nullptr;
		std::unordered_map<std::string, SymbolInfo>  symbols;
		std::vector<std::unique_ptr<Scope>>          children;

		[[nodiscard]] bool contains(size_t line, size_t column) const;
	};

	struct Occurrence
	{
		size_t      line = 0;
		size_t      startColumn = 0;
		size_t      endColumn = 0;
		std::string name;
		AST::Node  *node = nullptr;
		SymbolInfo *symbol = nullptr; ///< Resolved declaration, or nullptr if unresolved.
		bool        isDeclaration = false;

		[[nodiscard]] bool contains(size_t queryLine, size_t queryColumn) const
		{
			return line == queryLine && queryColumn >= startColumn && queryColumn < endColumn;
		}
	};

	struct Location
	{
		std::string uri;
		size_t      line;
		size_t      column;
	};

	struct TextEdit
	{
		size_t      line;
		size_t      startColumn;
		size_t      endColumn; ///< Exclusive.
		std::string newText;
	};

	struct CompletionItem
	{
		std::string label;
		SymbolKind  kind = SymbolKind::Variable;
		bool        isKeyword = false; ///< true for keyword completions (var, if, ...).
		std::string detail;            ///< sig/type shown alongside the label.
	};

	struct SignatureHelpResult
	{
		std::string              label; ///< e.g. "fn add(a: int, b: int) -> int".
		std::vector<std::string> paramLabels;
		int                       activeParameter = 0;
	};

	struct DocumentSymbolInfo
	{
		std::string                          name;
		SymbolKind                           kind = SymbolKind::Variable;
		std::string                          detail;
		size_t                                line = 0, column = 0;
		std::vector<DocumentSymbolInfo>      children;
	};

	struct DocumentState
	{
		std::string                                       uri;
		std::string                                       source;
		std::vector<size_t>                               lineStartOffsets;
		std::unique_ptr<AST::Program>                     program;
		std::vector<Diagnostic>                           diagnostics;
		std::unique_ptr<Scope>                            rootScope;
		std::vector<Occurrence>                           occurrences;
		std::unordered_map<std::string, AST::StructDecl *> structTypes; ///< type -> declaration.
		std::unordered_map<std::string, SymbolInfo>       fieldSymbols; ///< "structName.fieldName" -> field.
	};

	LSP() = default;
	~LSP() = default;

	void setIncludePaths(const std::vector<std::filesystem::path> &paths)
	{
		includePaths = paths;
	}

	void openDocument(const std::string &uri, const std::string &text);
	void changeDocument(const std::string &uri, const std::string &newText);
	void closeDocument(const std::string &uri);

	std::vector<Diagnostic> getDiagnostics(const std::string &uri) const;

	AST::Node *findNodeAtPosition(const std::string &uri, size_t line, size_t column);

	std::optional<std::string> getHover(const std::string &uri, size_t line, size_t column);

	std::optional<Location> getDefinition(const std::string &uri, size_t line, size_t column);

	std::vector<Location> getReferences(const std::string &uri, size_t line, size_t column,
	                                     bool includeDeclaration = true);

	std::optional<std::vector<TextEdit>> getRenameEdits(const std::string &uri, size_t line, size_t column,
	                                                      const std::string &newName);

	std::vector<CompletionItem> getCompletions(const std::string &uri, size_t line, size_t column);

	std::optional<SignatureHelpResult> getSignatureHelp(const std::string &uri, size_t line, size_t column);

	std::vector<DocumentSymbolInfo> getDocumentSymbols(const std::string &uri);

  private:
	std::unordered_map<std::string, DocumentState> documents;
	std::vector<std::filesystem::path>              includePaths;

	void        compile(DocumentState &doc);
	static void        computeLineOffsets(DocumentState &doc);
	static void        buildIndex(DocumentState &doc);
	static std::string symbolNameAt(AST::Node *node) ;

	/// @brief converts an LSP document URI (e.g. "file:///home/src/foo.phs") into a filesystem path
	static std::filesystem::path uriToPath(const std::string &uri);

	static const Occurrence *occurrenceAt(const DocumentState &doc, size_t line, size_t column) ;
	static std::string       renderHover(const SymbolInfo &sym) ;
};
} // namespace Phasor