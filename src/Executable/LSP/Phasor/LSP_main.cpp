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

#include "../../../LSP/Phasor/LSP.hpp"
#include <Value.hpp>
#include <functional>
#include <stdexcept>
#include <filesystem>
#include <sstream>
#include <nativeerror.h>
#include <phs_dupenv.hpp>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

static Phasor::string readMessage()
{
	size_t contentLength = 0;

	while (true)
	{
		std::string line;
		if (!std::getline(std::cin, line))
		{
			return "";
		}

		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}

		if (line.empty())
		{
			break;
		}

		const std::string prefix = "Content-Length: ";
		if (line.starts_with(prefix))
		{
			contentLength = std::stoull(line.substr(prefix.size()));
		}
	}

	if (contentLength == 0)
	{
		return "";
	}

	Phasor::string body(contentLength, '\0');
	std::cin.read(body.data(), static_cast<std::streamsize>(contentLength));
	return body;
}

static void writeMessage(const Phasor::Value &msg)
{
	const Phasor::string body = msg.jsonSerialize();
	std::print("Content-Length: {}\r\n\r\n{}", body.size(), body.c_str());
	std::fflush(stdout);
}

static Phasor::Value makeResponse(const Phasor::Value &id, Phasor::Value result)
{
	return {
	    {"jsonrpc", "2.0"}, 
	    {"id", id}, 
	    {"result", std::move(result)}
	};
}

static Phasor::Value makeError(const Phasor::Value &id, int code, const Phasor::string &message)
{
	return {
	    {"jsonrpc", "2.0"}, 
	    {"id", id}, 
	    {"error", {
	        {"code", code}, 
	        {"message", message}
	    }}
	};
}

static Phasor::Value makeNotification(const Phasor::string &method, Phasor::Value params)
{
	return {
	    {"jsonrpc", "2.0"}, 
	    {"method", method}, 
	    {"params", std::move(params)}
	};
}

static void publishDiagnostics(const Phasor::string &uri, const std::vector<Phasor::LSP::Diagnostic> &diags)
{
	auto arr = Phasor::Value::createArray();
	for (const auto &d : diags)
	{
		arr.asArray()->push_back({
		    {"range", {
		         {"start",{{"line", (Phasor::i64)d.startLine}, {"character", (Phasor::i64)d.startColumn}}},
		         {"end", {{"line", (Phasor::i64)d.endLine}, {"character", (Phasor::i64)d.endColumn}}}
		     }},
		    {"severity", 1},
		    {"message", d.message}
		});
	}
	writeMessage(makeNotification("textDocument/publishDiagnostics", {{"uri", uri}, {"diagnostics", std::move(arr)}}));
}

static Phasor::Value makePointRange(size_t line, size_t col)
{
	return {
	    {"start", {{"line", (Phasor::i64)line}, {"character", (Phasor::i64)col}}}, 
	    {"end", {{"line", (Phasor::i64)line}, {"character", (Phasor::i64)col + 1}}}
	};
}

static std::vector<std::filesystem::path> buildIncludePaths(const std::vector<std::filesystem::path> &clientPaths)
{
	std::vector<std::filesystem::path> paths;

#ifdef PHASOR_DEFAULT_FIRST_PATH
	paths.emplace_back(PHASOR_DEFAULT_FIRST_PATH);
#endif

	for (const auto &p : clientPaths)
	{
		paths.push_back(p);
	}

	Phasor::string includeDirs;
	if (Phasor::dupenv_ret ret = Phasor::dupenv(includeDirs, "PHASOR_INCLUDE_PATH"); ret == Phasor::dupenv_ret::Success)
	{
		std::stringstream ss(includeDirs.c_str());
		std::string       item;
		while (std::getline(ss, item, ';'))
		{
			if (!item.empty())
			{
				paths.emplace_back(item);
			}
		}
	}

	return paths;
}

static Phasor::Value handleInitialize(Phasor::LSP &lsp, const Phasor::Value &params)
{
	std::vector<std::filesystem::path> clientPaths;
	if (params.isStruct() && params.hasField("initializationOptions"))
	{
		auto initOpts = params.get_or("initializationOptions", phsnull);
		if (initOpts.isStruct() && initOpts.hasField("includePaths"))
		{
			auto incPaths = initOpts.get_or("includePaths", phsnull);
			if (incPaths.isArray())
			{
				const auto& arr = *incPaths.asArray();
				for (const auto &p : arr)
				{
					if (p.isString())
					{
						clientPaths.emplace_back(p.stl_string());
					}
				}
			}
		}
	}
	lsp.setIncludePaths(buildIncludePaths(clientPaths));

	return {
	    {"capabilities", {
	         {"textDocumentSync", 1},
	         {"hoverProvider", true},
	         {"definitionProvider", true},
	         {"referencesProvider", true},
	         {"renameProvider", true},
	         {"documentSymbolProvider", true},
	         {"completionProvider", {
	             {"triggerCharacters", Phasor::Value::createArray({"."})}
	         }},
	         {"signatureHelpProvider", {
	             {"triggerCharacters", Phasor::Value::createArray({"(", ","})}
	         }}
	     }},
	    {"serverInfo", {
	        {"name", "phasor-lsp"}, 
	        {"version", PHASOR_VERSION_STRING}
	    }}
	};
}

static Phasor::Value handleHover(Phasor::LSP &lsp, const Phasor::Value &params)
{
	const Phasor::string uri = params.get_or("textDocument", phsnull).get_or("uri", phsnull).string();
	const size_t      line = params.get_or("position", phsnull).get_or("line", phsnull).asInt();
	const size_t      col = params.get_or("position", phsnull).get_or("character", phsnull).asInt();

	auto text = lsp.getHover(uri, line, col);
	if (!text.has_value())
	{
		return phsnull;
	}

	return {
	    {"contents", {
	        {"kind", "markdown"}, 
	        {"value", "```phasor\n" + *text + "\n```"}
	    }},
	    {"range", makePointRange(line, col)}
	};
}

static Phasor::Value handleDefinition(Phasor::LSP &lsp, const Phasor::Value &params)
{
	const Phasor::string uri = params.get_or("textDocument", phsnull).get_or("uri", phsnull).string();
	const size_t      line = params.get_or("position", phsnull).get_or("line", phsnull).asInt();
	const size_t      col = params.get_or("position", phsnull).get_or("character", phsnull).asInt();

	auto loc = lsp.getDefinition(uri, line, col);
	if (!loc.has_value())
	{
		return phsnull;
	}

	return {
	    {"uri", loc->uri}, 
	    {"range", makePointRange(loc->line, loc->column)}
	};
}

static int toCompletionItemKind(Phasor::LSP::SymbolKind kind)
{
	switch (kind)
	{
	case Phasor::LSP::SymbolKind::Function:
	case Phasor::LSP::SymbolKind::ForwardDecl:
		return 3; // Function
	case Phasor::LSP::SymbolKind::Struct:
		return 22; // Struct
	case Phasor::LSP::SymbolKind::Field:
		return 5; // Field
	case Phasor::LSP::SymbolKind::Variable:
	case Phasor::LSP::SymbolKind::Parameter:
	default:
		return 6; // Variable
	}
}

static int toDocumentSymbolKind(Phasor::LSP::SymbolKind kind)
{
	switch (kind)
	{
	case Phasor::LSP::SymbolKind::Function:
	case Phasor::LSP::SymbolKind::ForwardDecl:
		return 12; // Function
	case Phasor::LSP::SymbolKind::Struct:
		return 23; // Struct
	case Phasor::LSP::SymbolKind::Field:
		return 8; // Field
	case Phasor::LSP::SymbolKind::Variable:
	case Phasor::LSP::SymbolKind::Parameter:
	default:
		return 13; // Variable
	}
}

static Phasor::Value handleReferences(Phasor::LSP &lsp, const Phasor::Value &params)
{
	const Phasor::string uri = params.get_or("textDocument", phsnull).get_or("uri", phsnull).string();
	const size_t             line = params.get_or("position", phsnull).get_or("line", phsnull).asInt();
	const size_t             col = params.get_or("position", phsnull).get_or("character", phsnull).asInt();
	bool                     includeDeclaration = true;
	
	if (params.contains("context") && params.get_or("context", phsnull).contains("includeDeclaration"))
	{
		includeDeclaration = params.get_or("context", phsnull).get_or("includeDeclaration", phsnull).asBool();
	}

	auto locs = lsp.getReferences(uri, line, col, includeDeclaration);
	auto arr = Phasor::Value::createArray();
	for (const auto &loc : locs)
	{
		arr.asArray()->push_back({
		    {"uri", loc.uri}, 
		    {"range", makePointRange(loc.line, loc.column)}
		});
	}
	return arr;
}

static Phasor::Value handleRename(Phasor::LSP &lsp, const Phasor::Value &params)
{
	const Phasor::string uri = params.get_or("textDocument", phsnull).get_or("uri", phsnull).string();
	const size_t             line = params.get_or("position", phsnull).get_or("line", phsnull).asInt();
	const size_t             col = params.get_or("position", phsnull).get_or("character", phsnull).asInt();
	const Phasor::string newName = params.get_or("newName", phsnull).string();

	auto edits = lsp.getRenameEdits(uri, line, col, newName);
	if (!edits.has_value())
	{
		return phsnull;
	}

	auto textEdits = Phasor::Value::createArray();
	for (const auto &e : *edits)
	{
		textEdits.asArray()->push_back({
		    {"range", {
		         {"start", {{"line", (Phasor::i64)e.line}, {"character", (Phasor::i64)e.startColumn}}},
		         {"end", {{"line", (Phasor::i64)e.line}, {"character", (Phasor::i64)e.endColumn}}}
		     }},
		    {"newText", e.newText}
		});
	}
	return {{"changes", {{uri, textEdits}}}};
}

static Phasor::Value handleCompletion(Phasor::LSP &lsp, const Phasor::Value &params)
{
	const Phasor::string uri = params.get_or("textDocument", phsnull).get_or("uri", phsnull).string();
	const size_t             line = params.get_or("position", phsnull).get_or("line", phsnull).asInt();
	const size_t             col = params.get_or("position", phsnull).get_or("character", phsnull).asInt();

	auto items = lsp.getCompletions(uri, line, col);
	auto arr = Phasor::Value::createArray();
	for (const auto &item : items)
	{
		Phasor::Value entry = {{"label", item.label}};
		if (item.isKeyword)
		{
			entry["kind"] = 14; // Keyword
		} else {
			entry["kind"] = toCompletionItemKind(item.kind);
			if (!item.detail.empty())
			{
				entry["detail"] = item.detail;
			}
		}
		arr.asArray()->push_back(std::move(entry));
	}
	return arr;
}

static Phasor::Value handleSignatureHelp(Phasor::LSP &lsp, const Phasor::Value &params)
{
	const Phasor::string uri = params.get_or("textDocument", phsnull).get_or("uri", phsnull).string();
	const size_t             line = params.get_or("position", phsnull).get_or("line", phsnull).asInt();
	const size_t             col = params.get_or("position", phsnull).get_or("character", phsnull).asInt();

	auto help = lsp.getSignatureHelp(uri, line, col);
	if (!help.has_value())
	{
		return phsnull;
	}

	auto params_ = Phasor::Value::createArray();
	for (const auto &p : help->paramLabels)
	{
		params_.asArray()->push_back({{"label", p}});
	}

	return {
	    {"signatures", Phasor::Value::createArray({
	         {
	             {"label", help->label}, 
	             {"parameters", params_}
	         }
	     })},
	    {"activeSignature", 0},
	    {"activeParameter", help->activeParameter}
	};
}

static Phasor::Value handleDocumentSymbol(Phasor::LSP &lsp, const Phasor::Value &params)
{
	const Phasor::string uri = params.get_or("textDocument", phsnull).get_or("uri", phsnull).string();

	std::function<Phasor::Value(const Phasor::LSP::DocumentSymbolInfo &)> toJson =
	    [&](const Phasor::LSP::DocumentSymbolInfo &sym) -> Phasor::Value
	{
		auto children = Phasor::Value::createArray();
		for (const auto &child : sym.children)
		{
			children.asArray()->push_back(toJson(child));
		}
		
		auto range = makePointRange(sym.line, sym.column);
		Phasor::Value entry = {
		    {"name", sym.name},
		    {"kind", toDocumentSymbolKind(sym.kind)},
		    {"range", range},
		    {"selectionRange", range}
		};
		
		if (!sym.detail.empty())
		{
			entry["detail"] = sym.detail;
		}
		if (!children.asArray()->empty())
		{
			entry["children"] = std::move(children);
		}
		
		return entry;
	};

	auto arr = Phasor::Value::createArray();
	for (const auto &sym : lsp.getDocumentSymbols(uri))
	{
		arr.asArray()->push_back(toJson(sym));
	}
	return arr;
}

int main()
{
#ifdef _WIN32
	_setmode(_fileno(stdout), _O_BINARY);
	_setmode(_fileno(stdin), _O_BINARY);
#endif
	std::ios::sync_with_stdio(false);

	Phasor::LSP lsp;
	lsp.setIncludePaths(buildIncludePaths({}));
	bool running = true;

	while (running)
	{
		const Phasor::string raw = readMessage();
		if (raw.empty())
		{
			break;
		}

		Phasor::Value msg;
		try
		{
			msg = Phasor::Value::from_json(raw);
		}
		catch (const std::exception &e)
		{
			writeMessage(makeError(phsnull, -32700, "JSON parse error: ") + e.what());
			continue;
		}

		const bool isRequest = msg.contains("id");
		const Phasor::Value id = isRequest ? msg.get_or("id", phsnull) : phsnull;
		
		Phasor::string method;
		if (msg.contains("method") && msg.get_or("method", phsnull).isString()) 
		{
			method = msg.get_or("method", phsnull).string();
		}
		
		const Phasor::Value params = msg.get_or("params", phsnull);

		if (method == "initialize")
		{
			writeMessage(makeResponse(id, handleInitialize(lsp, params)));
		}
		else if (method == "initialized")
		{
		}
		else if (method == "shutdown")
		{
			writeMessage(makeResponse(id, phsnull));
			running = false;
		}
		else if (method == "exit")
		{
			break;
		}
		else if (method == "textDocument/didOpen")
		{
			const Phasor::string uri = params.get_or("textDocument", phsnull).get_or("uri", phsnull).string();
			const Phasor::string text = params.get_or("textDocument", phsnull).get_or("text", phsnull).string();
			
			lsp.openDocument(uri, text);
			publishDiagnostics(uri, lsp.getDiagnostics(uri));
		}
		else if (method == "textDocument/didChange")
		{
			const Phasor::string uri = params.get_or("textDocument", phsnull).get_or("uri", phsnull).string();
			auto contentChanges = params.get_or("contentChanges", phsnull);
			
			if (contentChanges.isArray() && !contentChanges.asArray()->empty())
			{
				const Phasor::string text = contentChanges[0].get_or("text", phsnull).string();
				lsp.changeDocument(uri, text);
				publishDiagnostics(uri, lsp.getDiagnostics(uri));
			}
		}
		else if (method == "textDocument/didClose")
		{
			const Phasor::string uri = params.get_or("textDocument", phsnull).get_or("uri", phsnull).string();
			
			lsp.closeDocument(uri);
			publishDiagnostics(uri, {});
		}
		else if (method == "textDocument/hover")
		{
			writeMessage(makeResponse(id, handleHover(lsp, params)));
		}
		else if (method == "textDocument/definition")
		{
			writeMessage(makeResponse(id, handleDefinition(lsp, params)));
		}
		else if (method == "textDocument/references")
		{
			writeMessage(makeResponse(id, handleReferences(lsp, params)));
		}
		else if (method == "textDocument/rename")
		{
			writeMessage(makeResponse(id, handleRename(lsp, params)));
		}
		else if (method == "textDocument/completion")
		{
			writeMessage(makeResponse(id, handleCompletion(lsp, params)));
		}
		else if (method == "textDocument/signatureHelp")
		{
			writeMessage(makeResponse(id, handleSignatureHelp(lsp, params)));
		}
		else if (method == "textDocument/documentSymbol")
		{
			writeMessage(makeResponse(id, handleDocumentSymbol(lsp, params)));
		}
		else if (isRequest)
		{
			writeMessage(makeError(id, -32601, "Method not found: " + method));
		}
	}

	return 0;
}