// Copyright 2025-2026 Daniel McGuire
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
#include <streambuf>
#include <istream>
#include <array>
#include <memory>
#include <algorithm>
#include <limits>
#include <mutex>

// Phasor stdlibcore system/pipe_streambuf

#if defined(_WIN32)
	#include <windows.h>
#else
	#include <unistd.h>
	#include <cerrno>
	#include <csignal>
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
#if !defined(_WIN32)
		static std::once_flag ignoreSigpipeOnce;
		std::call_once(ignoreSigpipeOnce, [] { ::signal(SIGPIPE, SIG_IGN); });
#endif
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

	bool broken() const { return broken_; }

protected:
	int_type underflow() override
	{
		if (!isReadEnd_ || !open_) return traits_type::eof();

#if defined(_WIN32)
		DWORD n = 0;
		for (;;)
		{
			if (ReadFile(handle_, buffer_.data(), (DWORD)buffer_.size(), &n, nullptr))
				break;

			return traits_type::eof();
		}
		if (n == 0) return traits_type::eof();
#else
		ssize_t n;
		for (;;)
		{
			n = ::read(handle_, buffer_.data(), buffer_.size());
			if (n < 0 && errno == EINTR) continue;
			break;
		}
		if (n <= 0) return traits_type::eof();
#endif
		setg(buffer_.data(), buffer_.data(), buffer_.data() + n);
		return traits_type::to_int_type(*gptr());
	}

	std::streamsize xsputn(const char* s, std::streamsize count) override
	{
		if (isReadEnd_ || !open_ || broken_ || count <= 0) return 0;

		std::streamsize totalWritten = 0;
		while (totalWritten < count)
		{
#if defined(_WIN32)
			DWORD toWrite = (DWORD)std::min<std::streamsize>(
				count - totalWritten,
				(std::streamsize)(std::numeric_limits<DWORD>::max)());

			DWORD written = 0;
			if (!WriteFile(handle_, s + totalWritten, toWrite, &written, nullptr))
			{
				DWORD err = GetLastError();
				if (err == ERROR_BROKEN_PIPE || err == ERROR_NO_DATA)
					broken_ = true;
				break;
			}
			if (written == 0) break;
			totalWritten += written;
#else
			ssize_t written = ::write(handle_, s + totalWritten, (size_t)(count - totalWritten));
			if (written < 0)
			{
				if (errno == EINTR) continue;
				if (errno == EPIPE) broken_ = true;
				break;
			}
			if (written == 0) break;
			totalWritten += written;
#endif
		}
		return totalWritten;
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
	bool                    broken_ = false;
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