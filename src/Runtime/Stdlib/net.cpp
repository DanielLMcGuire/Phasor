#include <mutex>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <phsint.hpp>
#include <version.h>

#include "StdLib.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#endif

namespace Phasor
{

namespace {
#if defined(_WIN32)
using native_socket_t = SOCKET;
constexpr native_socket_t kInvalidSocket = INVALID_SOCKET;
inline void platformCloseSocket(native_socket_t s) { ::closesocket(s); }
#else
using native_socket_t = int;
constexpr native_socket_t kInvalidSocket = -1;
inline void platformCloseSocket(native_socket_t s) { ::close(s); }
#endif

class SocketStreamBuf : public std::streambuf
{
  public:
	explicit SocketStreamBuf(native_socket_t sock) : sock_(sock)
	{
		setg(getBuf_, getBuf_, getBuf_);
	}

	~SocketStreamBuf() override
	{
		if (sock_ != kInvalidSocket) platformCloseSocket(sock_);
	}

	SocketStreamBuf(const SocketStreamBuf &) = delete;
	SocketStreamBuf &operator=(const SocketStreamBuf &) = delete;

	native_socket_t nativeSocket() const { return sock_; }

  protected:
	int_type underflow() override
	{
		if (gptr() < egptr())
			return traits_type::to_int_type(*gptr());

		auto n = ::recv(sock_, getBuf_, static_cast<int>(sizeof(getBuf_)), 0);
		if (n <= 0)
			return traits_type::eof();

		setg(getBuf_, getBuf_, getBuf_ + n);
		return traits_type::to_int_type(*gptr());
	}

	std::streamsize xsputn(const char *s, std::streamsize count) override
	{
		std::streamsize total = 0;
		while (total < count)
		{
			auto n = ::send(sock_, s + total, static_cast<int>(count - total), 0);
			if (n <= 0) break;
			total += n;
		}
		return total;
	}

	int_type overflow(int_type ch) override
	{
		if (traits_type::eq_int_type(ch, traits_type::eof()))
			return traits_type::not_eof(ch);
		char c = traits_type::to_char_type(ch);
		return xsputn(&c, 1) == 1 ? ch : traits_type::eof();
	}

	int sync() override { return 0; }

  private:
	native_socket_t sock_ = kInvalidSocket;
	char            getBuf_[4096];
};

class SocketStream : public std::iostream
{
  public:
	explicit SocketStream(native_socket_t sock) : std::iostream(nullptr), buf_(sock)
	{
		rdbuf(&buf_);
	}
	native_socket_t nativeSocket() const { return buf_.nativeSocket(); }

  private:
	SocketStreamBuf buf_;
};

Phasor::i64 registerConnectedSocket(native_socket_t sock)
{
	auto stream = std::make_unique<SocketStream>(sock);
	return Phasor::StdLib::allocFileDescriptor(std::move(stream), Phasor::StdLib::StreamKind::Socket);
}

Phasor::i64 tcpConnectImpl(const Phasor::PhsString &host, const Phasor::PhsString &port, Phasor::i64 timeoutMs)
{
	Phasor::StdLib::ensureNetworkingInitialized();

	addrinfo hints{};
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	addrinfo *res = nullptr;
	if (::getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0 || !res)
		return -1;

	native_socket_t sock = kInvalidSocket;
	for (auto *cur = res; cur; cur = cur->ai_next)
	{
		sock = ::socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
		if (sock == kInvalidSocket) continue;

		if (timeoutMs >= 0)
		{
#if defined(_WIN32)
			DWORD tv = static_cast<DWORD>(timeoutMs);
			::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&tv), sizeof(tv));
			::setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&tv), sizeof(tv));
#else
			timeval tv{static_cast<long>(timeoutMs / 1000), static_cast<long>((timeoutMs % 1000) * 1000)};
			::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
			::setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
		}

		if (::connect(sock, cur->ai_addr, static_cast<int>(cur->ai_addrlen)) == 0)
			break;

		platformCloseSocket(sock);
		sock = kInvalidSocket;
	}
	::freeaddrinfo(res);

	if (sock == kInvalidSocket)
		return -1;

	return registerConnectedSocket(sock);
}

bool waitReadable(native_socket_t sock, Phasor::i64 timeoutMs)
{
#if defined(_WIN32)
	fd_set set;
	FD_ZERO(&set);
	FD_SET(sock, &set);
	timeval tv{static_cast<long>(timeoutMs / 1000), static_cast<long>((timeoutMs % 1000) * 1000)};
	return ::select(0, &set, nullptr, nullptr, &tv) > 0;
#else
	pollfd pfd{sock, POLLIN, 0};
	return ::poll(&pfd, 1, static_cast<int>(timeoutMs)) > 0;
#endif
}

PhsString readLine(std::iostream &s)
{
	std::string line;
	std::getline(s, line);
	if (!line.empty() && line.back() == '\r') line.pop_back();
	return line;
}

struct ParsedUrl { Phasor::PhsString scheme, host, path; int port = 80; };

bool parseUrl(const PhsString &url, ParsedUrl &out)
{
	auto schemeEnd = url.find("://");
	if (schemeEnd == std::string::npos) return false;
	out.scheme    = url.substr(0, schemeEnd);
	auto rest     = url.substr(schemeEnd + 3);
	auto pathStart = rest.find('/');
	PhsString authority = pathStart == std::string::npos ? rest : rest.substr(0, pathStart);
	out.path = pathStart == std::string::npos ? "/" : rest.substr(pathStart);
	if (authority.empty()) return false;

	auto colon = authority.find(':');
	if (colon == std::string::npos)
	{
		out.host = authority;
		out.port = (out.scheme == "https") ? 443 : 80;
	}
	else
	{
		out.host = authority.substr(0, colon);
		out.port = std::atoi(authority.c_str() + colon + 1);
	}
	return !out.host.empty();
}

Phasor::Value makeStruct(std::initializer_list<std::pair<Phasor::PhsString, Phasor::Value>> fields)
{
	return Phasor::Value(fields);
}

} // namespace

void StdLib::ensureNetworkingInitialized()
{
#if defined(_WIN32)
	static std::once_flag once;
	std::call_once(once, []
	{
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
	});
#endif
}

std::mutex& StdLib::getSocketPoolMutex()
{
	static std::mutex m;
	return m;
}

std::vector<StdLib::SocketEntry>& StdLib::getSocketPool()
{
	static std::vector<StdLib::SocketEntry> pool;
	return pool;
}

i64 StdLib::allocSocketHandle(std::unique_ptr<SocketHandle> h)
{
	std::lock_guard<std::mutex> lock(getSocketPoolMutex());
	auto &pool = getSocketPool();
	for (size_t i = 0; i < pool.size(); ++i)
		if (!pool[i].handle) { pool[i].handle = std::move(h); return static_cast<i64>(i); }
	pool.push_back({std::move(h)});
	return static_cast<i64>(pool.size() - 1);
}

StdLib::SocketHandle* StdLib::getSocketHandleLocked(i64 h)
{
	auto &pool = getSocketPool();
	if (h >= 0 && std::cmp_less(h, pool.size()) && pool[h].handle)
		return pool[h].handle.get();
	return nullptr;
}

void StdLib::closeSocketHandleLocked(i64 h)
{
	auto &pool = getSocketPool();
	if (h < 0 || std::cmp_greater_equal(h, pool.size()) || !pool[h].handle) return;
	auto &handle = *pool[h].handle;
	if (!handle.closed)
	{
		platformCloseSocket(static_cast<native_socket_t>(handle.nativeSocket));
		handle.closed = true;
	}
	pool[h].handle.reset();
}

void StdLib::registerNetFunctions(VM *vm)
{
	vm->registerNativeFunction("net_connect", net_connect);
	vm->registerNativeFunction("net_listen", net_listen);
	vm->registerNativeFunction("net_accept", net_accept);
	vm->registerNativeFunction("net_close_listener", net_close_listener);
	vm->registerNativeFunction("net_set_timeout", net_set_timeout);
	vm->registerNativeFunction("net_set_option", net_set_option);
	vm->registerNativeFunction("net_shutdown", net_shutdown);
	vm->registerNativeFunction("net_peer_address", net_peer_address);
	vm->registerNativeFunction("net_local_address", net_local_address);
	vm->registerNativeFunction("net_resolve", net_resolve);

	vm->registerNativeFunction("net_udp_open", net_udp_open);
	vm->registerNativeFunction("net_udp_send_to", net_udp_send_to);
	vm->registerNativeFunction("net_udp_recv_from", net_udp_recv_from);
	vm->registerNativeFunction("net_udp_close", net_udp_close);

    registerHttpFunctions(vm);
}

void StdLib::registerHttpFunctions(VM *vm)
{
    vm->registerNativeFunction("http_request", http_request);
}

Value StdLib::net_connect(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "net_connect", true);
	requireString(args[0], "net_connect", "1st argument (host)");
	requireInt(args[1], "net_connect", "2nd argument (port)");

	i64 timeoutMs = args.size() >= 3 ? args[2].asInt() : -1;
	i64 fd = tcpConnectImpl(args[0].stl_string(), std::to_string(args[1].asInt()), timeoutMs);
	return fd < 0 ? phsnull : Value(fd);
}

Value StdLib::net_listen(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "net_listen", true);
	requireString(args[0], "net_listen", "1st argument (host)");
	requireInt(args[1], "net_listen", "2nd argument (port)");

	ensureNetworkingInitialized();

	PhsString host = args[0].string();
	if (host.empty() || host == "*") host = "0.0.0.0";
	PhsString portStr = std::to_string(args[1].asInt());
	int backlog = args.size() >= 3 ? static_cast<int>(args[2].asInt()) : 128;

	addrinfo hints{};
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;
	addrinfo *res = nullptr;
	if (::getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) != 0 || !res)
		return phsnull;

	native_socket_t sock = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sock != kInvalidSocket)
	{
		int yes = 1;
		::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&yes), sizeof(yes));
	}

	if (sock == kInvalidSocket || ::bind(sock, res->ai_addr, static_cast<int>(res->ai_addrlen)) != 0 ||
	    ::listen(sock, backlog) != 0)
	{
		if (sock != kInvalidSocket) platformCloseSocket(sock);
		::freeaddrinfo(res);
		return phsnull;
	}
	::freeaddrinfo(res);

	auto handle = std::make_unique<SocketHandle>();
	handle->nativeSocket = static_cast<std::uintptr_t>(sock);
	handle->protocol     = SocketProtocol::TCP;
	handle->role         = SocketRole::Listener;
	handle->boundHost    = host;
	handle->boundPort    = static_cast<int>(args[1].asInt());

	return Value(allocSocketHandle(std::move(handle)));
}

Value StdLib::net_accept(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "net_accept", true);
	requireInt(args[0], "net_accept", "1st argument (listener handle)");

	native_socket_t listenSock = kInvalidSocket;
	{
		std::lock_guard<std::mutex> lock(getSocketPoolMutex());
		auto *handle = getSocketHandleLocked(args[0].asInt());
		if (!handle || handle->role != SocketRole::Listener) return phsnull;
		listenSock = static_cast<native_socket_t>(handle->nativeSocket);
	}

	if (args.size() >= 2)
	{
		requireInt(args[1], "net_accept", "2nd argument (timeoutMs)");
		if (!waitReadable(listenSock, args[1].asInt()))
			return phsnull;
	}

	sockaddr_storage addr{};
	socklen_t addrLen = sizeof(addr);
	native_socket_t client = ::accept(listenSock, reinterpret_cast<sockaddr *>(&addr), &addrLen);
	if (client == kInvalidSocket) return phsnull;

	char hostBuf[NI_MAXHOST]{};
	char portBuf[NI_MAXSERV]{};
	::getnameinfo(reinterpret_cast<sockaddr *>(&addr), addrLen, hostBuf, sizeof(hostBuf), portBuf,
	              sizeof(portBuf), NI_NUMERICHOST | NI_NUMERICSERV);

	i64 fd = registerConnectedSocket(client);

	return makeStruct({
		{"fd",   Value(fd)},
		{"host", Value(PhsString(hostBuf))},
		{"port", Value(static_cast<i64>(std::atoi(portBuf)))},
	});
}

bool StdLib::net_close_listener(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "net_close_listener", false);
	requireInt(args[0], "net_close_listener", "1st argument (listener handle)");
	std::lock_guard<std::mutex> lock(getSocketPoolMutex());
	closeSocketHandleLocked(args[0].asInt());
	return true;
}

bool StdLib::net_set_timeout(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "net_set_timeout", false);
	requireInt(args[0], "net_set_timeout", "1st argument (fd)");
	requireInt(args[1], "net_set_timeout", "2nd argument (ms)");

	auto *stream = getFileDescriptor(args[0].asInt());
	if (!stream) return false;
	auto *sockBuf = dynamic_cast<SocketStreamBuf *>(stream->rdbuf());
	if (!sockBuf) return false;

	native_socket_t sock = sockBuf->nativeSocket();
	i64 ms = args[1].asInt();
#if defined(_WIN32)
	DWORD tv = static_cast<DWORD>(ms);
	::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&tv), sizeof(tv));
	::setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&tv), sizeof(tv));
#else
	timeval tv{static_cast<long>(ms / 1000), static_cast<long>((ms % 1000) * 1000)};
	::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	::setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
	return true;
}

bool StdLib::net_set_option(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "net_set_option", false);
	requireInt(args[0], "net_set_option", "1st argument (fd)");
	requireString(args[1], "net_set_option", "2nd argument (option name)");

	auto *stream = getFileDescriptor(args[0].asInt());
	if (!stream) return false;
	auto *sockBuf = dynamic_cast<SocketStreamBuf *>(stream->rdbuf());
	if (!sockBuf) return false;
	native_socket_t sock = sockBuf->nativeSocket();

	PhsString opt = args[1].string();
	int value = args[2].isBool() ? (args[2].asBool() ? 1 : 0) : static_cast<int>(args[2].asInt());

	if (opt == "nodelay")
		return ::setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&value), sizeof(value)) == 0;
	if (opt == "keepalive")
		return ::setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char *>(&value), sizeof(value)) == 0;

	PHS_ERROR("net_set_option() unknown option '" + opt + "'");
}

bool StdLib::net_shutdown(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "net_shutdown", true);
	requireInt(args[0], "net_shutdown", "1st argument (fd)");

	auto *stream = getFileDescriptor(args[0].asInt());
	if (!stream) return false;
	auto *sockBuf = dynamic_cast<SocketStreamBuf *>(stream->rdbuf());
	if (!sockBuf) return false;

	PhsString how = args.size() >= 2 ? args[1].string() : "both";
#if defined(_WIN32)
	int mode = how == "read" ? SD_RECEIVE : how == "write" ? SD_SEND : SD_BOTH;
#else
	int mode = how == "read" ? SHUT_RD : how == "write" ? SHUT_WR : SHUT_RDWR;
#endif
	return ::shutdown(sockBuf->nativeSocket(), mode) == 0;
}

namespace {
Value socketAddressToValue(const sockaddr_storage &addr, socklen_t len)
{
	char hostBuf[NI_MAXHOST]{};
	char portBuf[NI_MAXSERV]{};
	::getnameinfo(reinterpret_cast<const sockaddr *>(&addr), len, hostBuf, sizeof(hostBuf), portBuf,
	              sizeof(portBuf), NI_NUMERICHOST | NI_NUMERICSERV);
	return makeStruct({
		{"host", Value(PhsString(hostBuf))},
		{"port", Value(static_cast<i64>(std::atoi(portBuf)))},
	});
}
} // namespace

Value StdLib::net_peer_address(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "net_peer_address", false);
	requireInt(args[0], "net_peer_address", "1st argument (fd)");

	auto *stream = getFileDescriptor(args[0].asInt());
	if (!stream) return phsnull;
	auto *sockBuf = dynamic_cast<SocketStreamBuf *>(stream->rdbuf());
	if (!sockBuf) return phsnull;

	sockaddr_storage addr{};
	socklen_t len = sizeof(addr);
	if (::getpeername(sockBuf->nativeSocket(), reinterpret_cast<sockaddr *>(&addr), &len) != 0) return phsnull;
	return socketAddressToValue(addr, len);
}

Value StdLib::net_local_address(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "net_local_address", false);
	requireInt(args[0], "net_local_address", "1st argument (fd)");

	auto *stream = getFileDescriptor(args[0].asInt());
	if (!stream) return phsnull;
	auto *sockBuf = dynamic_cast<SocketStreamBuf *>(stream->rdbuf());
	if (!sockBuf) return phsnull;

	sockaddr_storage addr{};
	socklen_t len = sizeof(addr);
	if (::getsockname(sockBuf->nativeSocket(), reinterpret_cast<sockaddr *>(&addr), &len) != 0) return phsnull;
	return socketAddressToValue(addr, len);
}

Value StdLib::net_resolve(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "net_resolve", false);
	requireString(args[0], "net_resolve", "1st argument (host)");
	ensureNetworkingInitialized();

	addrinfo hints{};
	hints.ai_family   = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	addrinfo *res = nullptr;
	if (::getaddrinfo(args[0].c_str(), nullptr, &hints, &res) != 0 || !res)
		return phsnull;

	Value::ArrayInstance ips;
	for (auto *cur = res; cur; cur = cur->ai_next)
	{
		char buf[INET6_ADDRSTRLEN]{};
		void *addr = cur->ai_family == AF_INET
			? static_cast<void *>(&reinterpret_cast<sockaddr_in *>(cur->ai_addr)->sin_addr)
			: static_cast<void *>(&reinterpret_cast<sockaddr_in6 *>(cur->ai_addr)->sin6_addr);
		::inet_ntop(cur->ai_family, addr, buf, sizeof(buf));
		ips.emplace_back(PhsString(buf));
	}
	::freeaddrinfo(res);
	return Value::createArray(ips);
}

Value StdLib::net_udp_open(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 0, "net_udp_open", true);
	ensureNetworkingInitialized();

	native_socket_t sock = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == kInvalidSocket) return phsnull;

	if (args.size() >= 2)
	{
		requireString(args[0], "net_udp_open", "1st argument (bind host)");
		requireInt(args[1], "net_udp_open", "2nd argument (bind port)");
		sockaddr_in addr{};
		addr.sin_family = AF_INET;
		addr.sin_port   = htons(static_cast<uint16_t>(args[1].asInt()));
		::inet_pton(AF_INET, args[0].c_str(), &addr.sin_addr);
		if (::bind(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0)
		{
			platformCloseSocket(sock);
			return phsnull;
		}
	}

	auto handle = std::make_unique<SocketHandle>();
	handle->nativeSocket = static_cast<std::uintptr_t>(sock);
	handle->protocol     = SocketProtocol::UDP;
	handle->role         = SocketRole::UdpSocket;

	return Value(allocSocketHandle(std::move(handle)));
}

i64 StdLib::net_udp_send_to(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 4, "net_udp_send_to", false);
	requireInt(args[0], "net_udp_send_to", "1st argument (handle)");
	requireString(args[1], "net_udp_send_to", "2nd argument (host)");
	requireInt(args[2], "net_udp_send_to", "3rd argument (port)");
	requireString(args[3], "net_udp_send_to", "4th argument (data)");

	native_socket_t sock = kInvalidSocket;
	{
		std::lock_guard<std::mutex> lock(getSocketPoolMutex());
		auto *handle = getSocketHandleLocked(args[0].asInt());
		if (!handle || handle->role != SocketRole::UdpSocket) return -1;
		sock = static_cast<native_socket_t>(handle->nativeSocket);
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(static_cast<uint16_t>(args[2].asInt()));
	::inet_pton(AF_INET, args[1].c_str(), &addr.sin_addr);

	PhsString data = args[3].string();
	auto sent = ::sendto(sock, data.data(), static_cast<int>(data.size()), 0,
	                      reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
	return static_cast<i64>(sent);
}

Value StdLib::net_udp_recv_from(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "net_udp_recv_from", true);
	requireInt(args[0], "net_udp_recv_from", "1st argument (handle)");
	requireInt(args[1], "net_udp_recv_from", "2nd argument (maxLen)");

	native_socket_t sock = kInvalidSocket;
	{
		std::lock_guard<std::mutex> lock(getSocketPoolMutex());
		auto *handle = getSocketHandleLocked(args[0].asInt());
		if (!handle || handle->role != SocketRole::UdpSocket) return phsnull;
		sock = static_cast<native_socket_t>(handle->nativeSocket);
	}

	if (args.size() >= 3)
	{
		requireInt(args[2], "net_udp_recv_from", "3rd argument (timeoutMs)");
		if (!waitReadable(sock, args[2].asInt()))
			return phsnull;
	}

	std::vector<char> buf(static_cast<size_t>(args[1].asInt()));
	sockaddr_in from{};
	socklen_t fromLen = sizeof(from);
	auto n = ::recvfrom(sock, buf.data(), static_cast<int>(buf.size()), 0,
	                     reinterpret_cast<sockaddr *>(&from), &fromLen);
	if (n <= 0) return phsnull;

	char hostBuf[INET_ADDRSTRLEN]{};
	::inet_ntop(AF_INET, &from.sin_addr, hostBuf, sizeof(hostBuf));

	return makeStruct({
		{"data", Value(PhsString(buf.data(), static_cast<size_t>(n)))},
		{"host", Value(PhsString(hostBuf))},
		{"port", Value(static_cast<i64>(ntohs(from.sin_port)))},
	});
}

bool StdLib::net_udp_close(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "net_udp_close", false);
	requireInt(args[0], "net_udp_close", "1st argument (handle)");
	std::lock_guard<std::mutex> lock(getSocketPoolMutex());
	closeSocketHandleLocked(args[0].asInt());
	return true;
}

Value StdLib::http_request(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "http_request", true);
	requireString(args[0], "http_request", "1st argument (method)");
	requireString(args[1], "http_request", "2nd argument (url)");

	ParsedUrl url;
	if (!parseUrl(args[1].stl_string(), url))
		PHS_ERROR("http_request() could not parse URL '" + args[1].stl_string() + "'");
	if (url.scheme == "https")
		PHS_ERROR("http_request() TLS/HTTPS is not supported yet -- use plain http:// for now");
	if (url.scheme != "http")
		PHS_ERROR("http_request() unsupported scheme '" + url.scheme + "'");

	PhsString body;
	if (args.size() >= 3 && args[2].isString()) body = args[2].string();

	i64 timeoutMs = args.size() >= 5 ? args[4].asInt() : 15000;

	i64 fd = tcpConnectImpl(url.host, std::to_string(url.port), timeoutMs);
	if (fd < 0)
		PHS_ERROR("http_request() failed to connect to " + url.host + ":" + std::to_string(url.port));

	auto* stream = getFileDescriptor(fd);

	*stream << args[0].stl_string() << " " << url.path << " HTTP/1.1\r\n";
	*stream << "Host: " << url.host << "\r\n";
	*stream << "Connection: close\r\n";
	*stream << std::format("User-Agent: phasor_language_{}\r\n", PHASOR_VERSION_STRING);

	if (args.size() >= 4 && args[3].isArray())
	{
		auto headerArr = args[3].asArray();
		for (const auto &headerVal : *headerArr)
		{
			if (!headerVal.isStruct()) continue;
			Value key = headerVal.get_or(PhsString("key"), phsnull);
			if (!key.isString()) continue;
			Value value = headerVal.get_or(PhsString("value"), phsnull);
			*stream << key.stl_string() << ": " << value.stl_string() << "\r\n";
		}
	}
	if (!body.empty())
		*stream << "Content-Length: " << body.size() << "\r\n";
	*stream << "\r\n";
	if (!body.empty())
		stream->write(body.data(), static_cast<std::streamsize>(body.size()));
	stream->flush();

	PhsString statusLine = readLine(*stream);
	int status = 0;
	if (auto firstSpace = statusLine.find(' '); firstSpace != std::string::npos)
		status = std::atoi(statusLine.c_str() + firstSpace + 1);

	std::vector<std::pair<PhsString, PhsString>> headers;
	long contentLength = -1;
	bool chunked = false;
	for (;;)
	{
		PhsString line = readLine(*stream);
		if (line.empty()) break;
		auto colon = line.find(':');
		if (colon == std::string::npos) continue;
		PhsString key = line.substr(0, colon);
		PhsString val = line.substr(colon + 1);
		while (!val.empty() && val.front() == ' ') val.erase(val.begin());
		headers.emplace_back(key, val);

		PhsString lowerKey = key;
		std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), [](unsigned char c) { return std::tolower(c); });
		if (lowerKey == "content-length") contentLength = std::atol(val.c_str());
		else if (lowerKey == "transfer-encoding" && val.find("chunked") != std::string::npos) chunked = true;
	}

	PhsString responseBody;
	if (chunked)
	{
		for (;;)
		{
			PhsString sizeLine = readLine(*stream);
			long chunkSize = std::strtol(sizeLine.c_str(), nullptr, 16);
			if (chunkSize <= 0) { readLine(*stream); break; }
			std::vector<char> chunk(static_cast<size_t>(chunkSize));
			stream->read(chunk.data(), chunkSize);
			responseBody.append(chunk.data(), static_cast<size_t>(stream->gcount()));
			readLine(*stream);
		}
	}
	else if (contentLength >= 0)
	{
		std::vector<char> buf(static_cast<size_t>(contentLength));
		stream->read(buf.data(), contentLength);
		responseBody.assign(buf.data(), static_cast<size_t>(stream->gcount()));
	}
	else
	{
		std::ostringstream oss;
		oss << stream->rdbuf();
		responseBody = PhsString(oss.str());
	}

	{
		auto &pool = getFilePool();
		if (fd >= 0 && std::cmp_less(fd, pool.size()))
			pool[fd].stream.reset();
	}

	Value::ArrayInstance headerPairs;
	for (auto &[k, v] : headers)
		headerPairs.emplace_back(makeStruct({{"key", Value(PhsString(k))}, {"value", Value(PhsString(v))}}));

	return makeStruct({
		{"status",  Value(static_cast<i64>(status))},
		{"headers", Value::createArray(headerPairs)},
		{"body",    Value(PhsString(responseBody))},
	});
}

} // namespace Phasor