#include "../../../LSP/Phasor/LSP.hpp"
#include <json.hpp>
#include <iostream>
#include <string>
#include <stdexcept>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

using json = nlohmann::json;

static std::string readMessage()
{
	size_t contentLength = 0;

	while (true)
	{
		std::string line;
		if (!std::getline(std::cin, line))
			return "";

		if (!line.empty() && line.back() == '\r')
			line.pop_back();

		if (line.empty())
			break;

		const std::string prefix = "Content-Length: ";
		if (line.rfind(prefix, 0) == 0)
			contentLength = std::stoull(line.substr(prefix.size()));
	}

	if (contentLength == 0)
		return "";

	std::string body(contentLength, '\0');
	std::cin.read(body.data(), static_cast<std::streamsize>(contentLength));
	return body;
}

static void writeMessage(const json &msg)
{
	const std::string body = msg.dump();
	std::cout << "Content-Length: " << body.size() << "\r\n\r\n" << body;
	std::cout.flush();
}

static json makeResponse(const json &id, json result)
{
	return {{"jsonrpc", "2.0"}, {"id", id}, {"result", std::move(result)}};
}

static json makeError(const json &id, int code, const std::string &message)
{
	return {{"jsonrpc", "2.0"}, {"id", id}, {"error", {{"code", code}, {"message", message}}}};
}

static json makeNotification(const std::string &method, json params)
{
	return {{"jsonrpc", "2.0"}, {"method", method}, {"params", std::move(params)}};
}

static void publishDiagnostics(const std::string &uri, const std::vector<Phasor::LSP::Diagnostic> &diags)
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

static json handleInitialize(const json &)
{
	return {{"capabilities", {
	          {"textDocumentSync", 1},
	          {"hoverProvider", true},
	          {"definitionProvider", true}}},
	        {"serverInfo", {{"name", "phasor-lsp"}, {"version", "0.1.0"}}}};
}

static json handleHover(Phasor::LSP &lsp, const json &params)
{
	const std::string uri = params["textDocument"]["uri"];
	const size_t      line = params["position"]["line"];
	const size_t      col = params["position"]["character"];

	auto text = lsp.getHover(uri, line, col);
	if (!text.has_value())
		return nullptr;

	return {{"contents",
	         {{"kind", "markdown"},
	          {"value", "```phasor\n" + *text + "\n```"}}},
	        {"range", makePointRange(line, col)}};
}

static json handleDefinition(Phasor::LSP &lsp, const json &params)
{
	const std::string uri = params["textDocument"]["uri"];
	const size_t      line = params["position"]["line"];
	const size_t      col = params["position"]["character"];

	auto loc = lsp.getDefinition(uri, line, col);
	if (!loc.has_value())
		return nullptr;

	return {{"uri", loc->uri}, {"range", makePointRange(loc->line, loc->column)}};
}

int main()
{
#ifdef _WIN32
	_setmode(_fileno(stdout), _O_BINARY);
	_setmode(_fileno(stdin), _O_BINARY);
#endif
	std::ios::sync_with_stdio(false);

	Phasor::LSP lsp;
	bool        running = true;

	while (running)
	{
		const std::string raw = readMessage();
		if (raw.empty())
			break;

		json msg;
		try
		{
			msg = json::parse(raw);
		}
		catch (const json::parse_error &e)
		{
			writeMessage(makeError(nullptr, -32700, "JSON parse error: " + std::string(e.what())));
			continue;
		}

		const bool        isRequest = msg.contains("id");
		const json        id = isRequest ? msg["id"] : json(nullptr);
		const std::string method = msg.value("method", "");
		const json       &params = msg.contains("params") ? msg["params"] : json(nullptr);

		if (method == "initialize")
		{
			writeMessage(makeResponse(id, handleInitialize(params)));
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
			const std::string uri = params["textDocument"]["uri"];
			const std::string text = params["textDocument"]["text"];
			lsp.openDocument(uri, text);
			publishDiagnostics(uri, lsp.getDiagnostics(uri));
		}
		else if (method == "textDocument/didChange")
		{
			const std::string uri = params["textDocument"]["uri"];
			if (params.contains("contentChanges") && !params["contentChanges"].empty())
			{
				const std::string text = params["contentChanges"][0]["text"];
				lsp.changeDocument(uri, text);
				publishDiagnostics(uri, lsp.getDiagnostics(uri));
			}
		}
		else if (method == "textDocument/didClose")
		{
			const std::string uri = params["textDocument"]["uri"];
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
		else if (isRequest)
		{
			writeMessage(makeError(id, -32601, "Method not found: " + method));
		}
	}

	return 0;
}