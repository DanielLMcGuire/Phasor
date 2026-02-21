#pragma once
#include "../../AST/AST.hpp"
#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

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

	struct SymbolInfo
	{
		std::string name;
		AST::Node  *declaration = nullptr;
		std::string signature;
	};

	struct Location
	{
		std::string uri;
		size_t      line;
		size_t      column;
	};

	struct DocumentState
	{
		std::string                   uri;
		std::string                   source;
		std::vector<size_t>           lineStartOffsets;
		std::unique_ptr<AST::Program> program;
		std::vector<Diagnostic>       diagnostics;
		std::unordered_map<std::string, SymbolInfo> globalSymbols;
	};

	LSP() = default;
	~LSP() = default;

	void openDocument(const std::string &uri, const std::string &text);
	void changeDocument(const std::string &uri, const std::string &newText);
	void closeDocument(const std::string &uri);

	std::vector<Diagnostic> getDiagnostics(const std::string &uri) const;

	AST::Node *findNodeAtPosition(const std::string &uri, size_t line, size_t column);

	std::optional<std::string> getHover(const std::string &uri, size_t line, size_t column);

	std::optional<Location> getDefinition(const std::string &uri, size_t line, size_t column);

  private:
	std::unordered_map<std::string, DocumentState> documents;

	void compile(DocumentState &doc);
	void computeLineOffsets(DocumentState &doc);
	void buildGlobalSymbols(DocumentState &doc);
	AST::Node *walkForNode(const DocumentState &doc, size_t line, size_t col);
	std::string symbolNameAt(AST::Node *node) const;
};
} // namespace Phasor