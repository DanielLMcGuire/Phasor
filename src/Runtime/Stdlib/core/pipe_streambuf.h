#pragma once
#include <streambuf>
#include <istream>
#include <array>
#include <memory>

#if defined(_WIN32)
	#include <windows.h>
#else
	#include <unistd.h>
#endif

namespace Phasor
{

class NativePipeStreamBuf : public std::streambuf
{
public:
#if defined(_WIN32)
	using native_handle_t = HANDLE;
#else
	using native_handle_t = int;
#endif

	NativePipeStreamBuf(native_handle_t handle, bool isReadEnd)
		: handle_(handle), isReadEnd_(isReadEnd)
	{
		if (isReadEnd_) setg(buffer_.data(), buffer_.data(), buffer_.data());
	}

	~NativePipeStreamBuf() override { close(); }

	void close()
	{
		if (!open_) return;
		open_ = false;
#if defined(_WIN32)
		if (handle_) CloseHandle(handle_);
#else
		if (handle_ >= 0) ::close(handle_);
#endif
	}

protected:
	int_type underflow() override
	{
		if (!isReadEnd_ || !open_) return traits_type::eof();
#if defined(_WIN32)
		DWORD n = 0;
		if (!ReadFile(handle_, buffer_.data(), (DWORD)buffer_.size(), &n, nullptr) || n == 0)
			return traits_type::eof();
#else
		ssize_t n = ::read(handle_, buffer_.data(), buffer_.size());
		if (n <= 0) return traits_type::eof();
#endif
		setg(buffer_.data(), buffer_.data(), buffer_.data() + n);
		return traits_type::to_int_type(*gptr());
	}

	std::streamsize xsputn(const char* s, std::streamsize count) override
	{
		if (isReadEnd_ || !open_) return 0;
#if defined(_WIN32)
		DWORD written = 0;
		if (!WriteFile(handle_, s, (DWORD)count, &written, nullptr)) return 0;
		return (std::streamsize)written;
#else
		ssize_t written = ::write(handle_, s, (size_t)count);
		return written < 0 ? 0 : (std::streamsize)written;
#endif
	}

	int_type overflow(int_type ch) override
	{
		if (isReadEnd_ || !open_ || ch == traits_type::eof()) return traits_type::not_eof(ch);
		char c = traits_type::to_char_type(ch);
		return xsputn(&c, 1) == 1 ? ch : traits_type::eof();
	}

private:
	native_handle_t         handle_;
	bool                    isReadEnd_;
	bool                    open_ = true;
	std::array<char, 4096>  buffer_{};
};

class OwningIOStream : public std::iostream
{
public:
	explicit OwningIOStream(std::unique_ptr<std::streambuf> buf)
		: std::iostream(buf.get()), buf_(std::move(buf)) {}
private:
	std::unique_ptr<std::streambuf> buf_;
};

} // namespace Phasor