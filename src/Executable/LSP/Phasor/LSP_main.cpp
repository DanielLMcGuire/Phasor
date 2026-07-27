#include "../../../LSP/Phasor/LSP.hpp"
#include <json.hpp>
#include <PhasorString.hpp>
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

using json = nlohmann::json;

static Phasor::PhsString readMessage()
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

		const Phasor::PhsString prefix = "Content-Length: ";
		if (line.starts_with(prefix))
		{
			contentLength = std::stoull(line.substr(prefix.size()));
		}
	}

	if (contentLength == 0)
	{
		return "";
	}

	Phasor::PhsString body(contentLength, '\0');
	std::cin.read(body.data(), static_cast<std::streamsize>(contentLength));
	return body;
}

static void writeMessage(const json &msg)
{
	const Phasor::PhsString body = msg.dump();
	std::print("Content-Length: {}\r\n\r\n{}", body.size(), body);
	std::fflush(stdout);
}

static json makeResponse(const json &id, json result)
{
	return {{"jsonrpc", "2.0"}, {"id", id}, {"result", std::move(result)}};
}

static json makeError(const json &id, int code, const Phasor::PhsString &message)
{
	return {{"jsonrpc", "2.0"}, {"id", id}, {"error", {{"code", code}, {"message", message}}}};
}

static json makeNotification(const Phasor::PhsString &method, json params)
{
	return {{"jsonrpc", "2.0"}, {"method", method}, {"params", std::move(params)}};
}

static void publishDiagnostics(const Phasor::PhsString &uri, const std::vector<Phasor::LSP::Diagnostic> &diags)
{
	json arr = json::array();
	for (const auto &d : diags)
	{
		arr.push_back({{"range",
		                {{"start", {{"line", d.startLine}, {"character", d.startColumn}}},
		                 {"end", {{"line", d.endLine}, {"character", d.endColumn}}}}},
		               {"severity", 1},
		               {"message", d.message}});
	}
	writeMessage(makeNotification("textDocument/publishDiagnostics", {{"uri", uri}, {"diagnostics", arr}}));
}

static json makePointRange(size_t line, size_t col)
{
	return {{"start", {{"line", line}, {"character", col}}}, {"end", {{"line", line}, {"character", col + 1}}}};
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

	Phasor::PhsString includeDirs;
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

static json handleInitialize(Phasor::LSP &lsp, const json &params)
{
	std::vector<std::filesystem::path> clientPaths;
	if (params.is_object() && params.contains("initializationOptions") &&
	    params["initializationOptions"].is_object() && params["initializationOptions"].contains("includePaths") &&
	    params["initializationOptions"]["includePaths"].is_array())
	{
		for (const auto &p : params["initializationOptions"]["includePaths"])
		{
			if (p.is_string())
			{
				clientPaths.emplace_back(p.get<std::string>());
			}
		}
	}
	lsp.setIncludePaths(buildIncludePaths(clientPaths));

	return {{"capabilities",
	         {{"textDocumentSync", 1},
	          {"hoverProvider", true},
	          {"definitionProvider", true},
	          {"referencesProvider", true},
	          {"renameProvider", true},
	          {"documentSymbolProvider", true},
	          {"completionProvider", {{"triggerCharacters", {"."}}}},
	          {"signatureHelpProvider", {{"triggerCharacters", {"(", ","}}}}}},
	        {"serverInfo", {{"name", "phasor-lsp"}, {"version", PHASOR_VERSION_STRING}}}};
}

static json handleHover(Phasor::LSP &lsp, const json &params)
{
	const Phasor::PhsString uri = std::string(params["textDocument"]["uri"]);
	const size_t      line = params["position"]["line"];
	const size_t      col = params["position"]["character"];

	auto text = lsp.getHover(uri, line, col);
	if (!text.has_value())
	{
		return nullptr;
	}

	return {{"contents", {{"kind", "markdown"}, {"value", "```phasor\n" + *text + "\n```"}}},
	        {"range", makePointRange(line, col)}};
}

static json handleDefinition(Phasor::LSP &lsp, const json &params)
{
	const Phasor::PhsString uri = std::string(params["textDocument"]["uri"]);
	const size_t      line = params["position"]["line"];
	const size_t      col = params["position"]["character"];

	auto loc = lsp.getDefinition(uri, line, col);
	if (!loc.has_value())
	{
		return nullptr;
	}

	return {{"uri", loc->uri}, {"range", makePointRange(loc->line, loc->column)}};
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

static json handleReferences(Phasor::LSP &lsp, const json &params)
{
	const Phasor::PhsString uri = std::string(params["textDocument"]["uri"]);
	const size_t             line = params["position"]["line"];
	const size_t             col = params["position"]["character"];
	bool                     includeDeclaration = true;
	if (params.contains("context") && params["context"].contains("includeDeclaration"))
	{
		includeDeclaration = params["context"]["includeDeclaration"].get<bool>();
	}

	auto locs = lsp.getReferences(uri, line, col, includeDeclaration);
	json arr = json::array();
	for (const auto &loc : locs)
	{
		arr.push_back({{"uri", loc.uri}, {"range", makePointRange(loc.line, loc.column)}});
	}
	return arr;
}

static json handleRename(Phasor::LSP &lsp, const json &params)
{
	const Phasor::PhsString uri = std::string(params["textDocument"]["uri"]);
	const size_t             line = params["position"]["line"];
	const size_t             col = params["position"]["character"];
	const Phasor::PhsString newName = std::string(params["newName"]);

	auto edits = lsp.getRenameEdits(uri, line, col, newName);
	if (!edits.has_value())
	{
		return nullptr;
	}

	json textEdits = json::array();
	for (const auto &e : *edits)
	{
		textEdits.push_back({{"range",
		                      {{"start", {{"line", e.line}, {"character", e.startColumn}}},
		                       {"end", {{"line", e.line}, {"character", e.endColumn}}}}},
		                     {"newText", e.newText}});
	}
	return {{"changes", {{uri, textEdits}}}};
}

static json handleCompletion(Phasor::LSP &lsp, const json &params)
{
	const Phasor::PhsString uri = std::string(params["textDocument"]["uri"]);
	const size_t             line = params["position"]["line"];
	const size_t             col = params["position"]["character"];

	auto items = lsp.getCompletions(uri, line, col);
	json arr = json::array();
	for (const auto &item : items)
	{
		json entry = {{"label", item.label}};
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
		arr.push_back(std::move(entry));
	}
	return arr;
}

static json handleSignatureHelp(Phasor::LSP &lsp, const json &params)
{
	const Phasor::PhsString uri = std::string(params["textDocument"]["uri"]);
	const size_t             line = params["position"]["line"];
	const size_t             col = params["position"]["character"];

	auto help = lsp.getSignatureHelp(uri, line, col);
	if (!help.has_value())
	{
		return nullptr;
	}

	json params_ = json::array();
	for (const auto &p : help->paramLabels)
	{
		params_.push_back({{"label", p}});
	}

	return {{"signatures", {{{"label", help->label}, {"parameters", params_}}}},
	        {"activeSignature", 0},
	        {"activeParameter", help->activeParameter}};
}

static json handleDocumentSymbol(Phasor::LSP &lsp, const json &params)
{
	const Phasor::PhsString uri = std::string(params["textDocument"]["uri"]);

	std::function<json(const Phasor::LSP::DocumentSymbolInfo &)> toJson =
	    [&](const Phasor::LSP::DocumentSymbolInfo &sym) -> json
	{
		json children = json::array();
		for (const auto &child : sym.children)
		{
			children.push_back(toJson(child));
		}
		json range = makePointRange(sym.line, sym.column);
		json entry = {{"name", sym.name},
		              {"kind", toDocumentSymbolKind(sym.kind)},
		              {"range", range},
		              {"selectionRange", range}};
		if (!sym.detail.empty())
		{
			entry["detail"] = sym.detail;
		}
		if (!children.empty())
		{
			entry["children"] = children;
		}
		return entry;
	};

	json arr = json::array();
	for (const auto &sym : lsp.getDocumentSymbols(uri))
	{
		arr.push_back(toJson(sym));
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
	bool        running = true;

	while (running)
	{
		const Phasor::PhsString raw = readMessage();
		if (raw.empty())
		{
			break;
		}

		json msg;
		try
		{
			msg = json::parse(raw);
		}
		catch (const json::parse_error &e)
		{
			writeMessage(makeError(nullptr, -32700, "JSON parse error: " + Phasor::PhsString(e.what())));
			continue;
		}

		const bool        isRequest = msg.contains("id");
		const json        id = isRequest ? msg["id"] : json(nullptr);
		const Phasor::PhsString method = msg.value("method", "");
		const json       &params = msg.contains("params") ? msg["params"] : json(nullptr);

		if (method == "initialize")
		{
			writeMessage(makeResponse(id, handleInitialize(lsp, params)));
		}
		else if (method == "initialized")
		{
		}
		else if (method == "shutdown")
		{
			writeMessage(makeResponse(id, nullptr));
			running = false;
		}
		else if (method == "exit")
		{
			break;
		}

		else if (method == "textDocument/didOpen")
		{
			const Phasor::PhsString uri = std::string(params["textDocument"]["uri"]);
			const Phasor::PhsString text = std::string(params["textDocument"]["text"]);
			lsp.openDocument(uri, text);
			publishDiagnostics(uri, lsp.getDiagnostics(uri));
		}
		else if (method == "textDocument/didChange")
		{
			const Phasor::PhsString uri = std::string(params["textDocument"]["uri"]);
			if (params.contains("contentChanges") && !params["contentChanges"].empty())
			{
				const Phasor::PhsString text = std::string(params["contentChanges"][0]["text"]);
				lsp.changeDocument(uri, text);
				publishDiagnostics(uri, lsp.getDiagnostics(uri));
			}
		}
		else if (method == "textDocument/didClose")
		{
			const Phasor::PhsString uri = std::string(params["textDocument"]["uri"]);
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