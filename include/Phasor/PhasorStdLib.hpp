/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                             //
//   PPPPPPP  H     H      AA      SSSSSSS  OOOOOOO  RRRRRRR    L            AA      NN    N  GGGGGGG  U     U      AA      GGGGGGG  EEEEEEE   //
//   P     P  H     H     A  A     S        O     O  R     R    L           A  A     N N   N  G        U     U     A  A     G        E         //
//   PPPPPPP  HHHHHHH    AAAAAA    SSSSSSS  O     O  RRRRRRR    L          AAAAAA    N  N  N  G  GGGG  U     U    AAAAAA    G  GGGG  EEEEEEE   //
//   P        H     H   A      A         S  O     O  R    R     L         A      A   N   N N  G     G  U     U   A      A   G     G  E         //
//   P        H     H  A        A  SSSSSSS  OOOOOOO  R     R    LLLLLLL  A        A  N    NN  GGGGGGG  UUUUUUU  A        A  GGGGGGG  EEEEEEE   //
//                                                                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with LLVM-Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://llvm.org/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// README
//
// Usage:
// ```cpp
// #include "PhasorStdLib.hpp"
// // later
// Phasor::StdLib::registerFunctions(vm);
// ```

#pragma once

#include <cmath>
#include <cstdlib>
#include <cstring>

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "PhasorVM.hpp"
#include "../Value.hpp"

#ifdef PHASOR_USES_BOOST
	#include <boost/assert/source_location.hpp>
	#define PHS_SRC_LOC() (std::format("{} @ {}:{}", BOOST_CURRENT_LOCATION.function_name(), BOOST_CURRENT_LOCATION.file_name(), BOOST_CURRENT_LOCATION.line()))
#else
	#define PHS_SRC_LOC() (std::format("StdLib::{}()", __func__))
#endif

#define PHS_ERROR(x) throw std::runtime_error(std::format("\"{}\" thrown in {}", x, PHS_SRC_LOC()));

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/// @brief Native function signature
using NativeFunction = std::function<Value(const Value::ArrayInstance &args, VM *vm)>;

/**
 * @class StdLib
 * @brief Phasor Standard library
 *
 * Contains all the base standard library functions used for any language targetting the Phasor Runtime
 */
class StdLib
{
  public:
	inline static void registerFunctions(VM &vm)
	{
#ifdef TRACING
		vm.log(std::format("({})(&VM@{:#x})\n", PHS_SRC_LOC(), reinterpret_cast<std::uintptr_t>(&vm)));
		vm.flush();
#endif
		vm.registerNativeFunction("phs__std", std_import);
#ifndef SANDBOXED
		vm.registerNativeFunction("assert", std_assert);
#endif
	}

	enum class StreamKind
	{
		File,
		Memory,
		Pipe,
		Socket
	};

	struct FileHandle
	{
		std::unique_ptr<std::iostream> stream;
		StreamKind                     kind = StreamKind::File;
		i64                            ownerProcess = -1;
	};

	static std::vector<FileHandle>& getFilePool();
	static i64 allocFileDescriptor(std::unique_ptr<std::iostream> stream,
	                                StreamKind kind = StreamKind::File,
	                                i64 ownerProcess = -1);
	static std::iostream* getFileDescriptor(i64 fd);

	struct ProcessHandle
	{
#if defined(_WIN32)
		void*         nativeHandle = nullptr;
		unsigned long processId    = 0;
#else
		long          pid = -1;
#endif
		bool exited    = false;
		int  exitCode  = -1;
		bool isolated  = false;
		bool forgotten = false;
	};
	struct ProcessEntry { std::unique_ptr<ProcessHandle> handle; };

	static std::mutex&                getProcessPoolMutex();
	static std::vector<ProcessEntry>& getProcessPool();
	static i64                        allocProcessHandle(std::unique_ptr<ProcessHandle> h);
	static ProcessHandle*             getProcessHandleLocked(i64 h); ///< assumes mutex already held
	static bool                       pollProcessExitLocked(ProcessHandle &proc); ///< assumes mutex already held
	static void                       releaseProcessHandleLocked(i64 h); ///< assumes mutex already held
	static void                       ensureReaperStarted();
	enum class SocketProtocol { TCP, UDP };
	enum class SocketRole     { Listener, UdpSocket };

	struct SocketHandle
	{
		std::uintptr_t nativeSocket = 0;
		SocketProtocol  protocol = SocketProtocol::TCP;
		SocketRole      role     = SocketRole::Listener;
		std::string     boundHost;
		int             boundPort = 0;
		bool            closed    = false;
	};
	struct SocketEntry { std::unique_ptr<SocketHandle> handle; };

	static std::mutex&               getSocketPoolMutex();
	static std::vector<SocketEntry>& getSocketPool();
	static i64                       allocSocketHandle(std::unique_ptr<SocketHandle> h);
	static SocketHandle*             getSocketHandleLocked(i64 h); ///< assumes mutex already held
	static void                      closeSocketHandleLocked(i64 h); ///< assumes mutex already held
	static void                      ensureNetworkingInitialized(); ///< WSAStartup on win32

	static void requireString(const Value &v, const char *fnName, const char *what);
	static void requireInt(const Value &v, const char *fnName, const char *what);
	static void requireBool(const Value &v, const char *fnName, const char *what);

	static char **argv; ///< Command line arguments
	static int    argc; ///< Number of command line arguments

	static void checkArgCount(const Value::ArrayInstance &args, size_t minimumArguments, const std::string &name,
	                          bool allowMoreArguments = false);
	static Phasor::i64 pointer_to_i64(void* ptr);
	static void* i64_to_pointer(Phasor::i64 value);
	static std::vector<Phasor::PhsString> phasorStringArrayToVector(const Phasor::Value &arr);
	static std::vector<char *> phasorStringArrayToCharArray(const Phasor::Value &arr, bool nullTerminate = false);

  private:
  	static std::unordered_map<PhsString, std::function<void(Phasor::VM *)>> modules;
	static std::unordered_map<PhsString, std::function<Value(const Value::ArrayInstance &args, VM *vm)>> functions;

	static bool  std_import(const Value::ArrayInstance &args, VM *vm);
#ifndef SANDBOXED
	static Value std_assert(const Value::ArrayInstance &args, VM *vm);
#endif

	static void registerMetaFunctions(VM *vm);
	static void registerMemoryFunctions(VM *vm);
	static void registerMathFunctions(VM *vm);
	static void registerRandomFunctions(VM *vm);
	static void registerStringFunctions(VM *vm);
	static void registerTypeConvFunctions(VM *vm);
#ifndef SANDBOXED
	static void registerFileFunctions(VM *vm);
	static void registerIniFunctions(VM* vm);
	static void registerNetFunctions(VM *vm);
	static void registerHttpFunctions(VM *vm);
#endif
	static void registerSysFunctions(VM *vm);
	static void registerIOFunctions(VM *vm);
	static void registerArrayFunctions(VM *vm);
	static void registerObjectFunctions(VM *vm);
	static void registerInternalFunctions(VM *vm);

#pragma region stdmeta
#ifndef SANDBOXED
	static i64       meta_operation(const Value::ArrayInstance &args, VM *vm);
	static Value     meta_stack_run(const Value::ArrayInstance &args, VM *vm);
	static Value     meta_push(const Value::ArrayInstance &args, VM *vm);
	static Value     meta_pop(const Value::ArrayInstance &args, VM *vm);
#endif
	static PhsString meta_get_version(const Value::ArrayInstance &args, VM *vm);
	static Value     meta_get_self(const Value::ArrayInstance &args, VM *vm);
	static Value     meta_get_registers(const Value::ArrayInstance &args, VM *vm);
	static Value     meta_load_bytecode_from_file(const Value::ArrayInstance &args, VM *vm);
	static bool      meta_save_bytecode_to_file(const Value::ArrayInstance &args, VM *vm);
	static i64       meta_new_state(const Value::ArrayInstance &args, VM *vm);
	static bool      meta_free_state(const Value::ArrayInstance &args, VM *vm);
	static i64       meta_eval_phs(const Value::ArrayInstance &args, VM *vm);
	static i64       meta_exec_phsb(const Value::ArrayInstance &args, VM *vm);
	static Value     meta_compile_phs(const Value::ArrayInstance &args, VM *vm);
#pragma endregion

#pragma region stdmemory
	static Value var_free(const Value::ArrayInstance &args, VM *vm); ///< Free a variable
	static i64 native_memory_malloc(const Value::ArrayInstance &args, VM *vm);
	static i64 native_memory_calloc(const Value::ArrayInstance &args, VM *vm);
	static i64 native_memory_realloc(const Value::ArrayInstance & args, VM *vm);
	static Value native_memory_write(const Value::ArrayInstance &args, VM *vm);
	static Value native_memory_write_offset(const Value::ArrayInstance &args, VM *vm);
	static Value native_memory_free(const Value::ArrayInstance &args, VM *vm);
	static Value native_memory_write_string(const Value::ArrayInstance &args, VM *vm);
	static Value native_memory_write_string_offset(const Value::ArrayInstance &args, VM *vm);
	static i64 native_memory_read(const Value::ArrayInstance &args, VM *vm);
	static PhsString native_memory_read_string(const Value::ArrayInstance &args, VM *vm);
	static i64 native_memory_read_offset(const Value::ArrayInstance &args, VM *vm);
	static PhsString native_memory_read_string_offset(const Value::ArrayInstance &args, VM *vm);
	static Value native_memory_strcpy(const Value::ArrayInstance &args, VM *vm);
	static Value native_memory_stralloc(const Value::ArrayInstance &args, VM *vm);
	static i64 native_memory_argv(const Value::ArrayInstance &args, VM *vm);
	static i64 native_memory_argc(const Value::ArrayInstance &args, VM *vm);
#pragma endregion

#pragma region stdmath
	static f64    math_sqrt(const Value::ArrayInstance &args, VM *vm);  ///< Square root
	static f64    math_pow(const Value::ArrayInstance &args, VM *vm);   ///< Power
	static Value  math_abs(const Value::ArrayInstance &args, VM *vm);   ///< Absolute value
	static f64    math_floor(const Value::ArrayInstance &args, VM *vm); ///< Floor
	static f64    math_ceil(const Value::ArrayInstance &args, VM *vm);  ///< Ceiling
	static f64    math_round(const Value::ArrayInstance &args, VM *vm); ///< Round
	static Value  math_min(const Value::ArrayInstance &args, VM *vm);   ///< Minimum
	static Value  math_max(const Value::ArrayInstance &args, VM *vm);   ///< Maximum
	static f64    math_log(const Value::ArrayInstance &args, VM *vm);   ///< Natural logarithm
	static f64    math_exp(const Value::ArrayInstance &args, VM *vm);   ///< Exponential
	static f64    math_sin(const Value::ArrayInstance &args, VM *vm);   ///< Sine
	static f64    math_cos(const Value::ArrayInstance &args, VM *vm);   ///< Cosine
	static f64    math_tan(const Value::ArrayInstance &args, VM *vm);   ///< Tangent
#pragma endregion

#pragma region stdfile
#ifndef SANDBOXED
	static Value     file_open(const Value::ArrayInstance &args, VM *vm); ///< Get file descriptor
	static bool      file_close(const Value::ArrayInstance &args, VM *vm); ///< Close file descriptor
	static PhsString file_absolute(const Value::ArrayInstance &args, VM *vm);   ///< Get full path to relative path
	static PhsString file_relative(const Value::ArrayInstance &args, VM *vm);   ///< Get relative path to given path
	static Value     file_read(const Value::ArrayInstance &args, VM *vm);       ///< Read file
	static bool      file_write(const Value::ArrayInstance &args, VM *vm);      ///< Write to file
	static bool      file_exists(const Value::ArrayInstance &args, VM *vm);     ///< Check if file exists
	static Value     file_read_line(const Value::ArrayInstance &args, VM *vm);  ///< Read a line from file
	static bool      file_write_line(const Value::ArrayInstance &args, VM *vm); ///< Write a line to file
	static bool      file_append(const Value::ArrayInstance &args, VM *vm);     ///< Append to file
	static bool      file_delete(const Value::ArrayInstance &args, VM *vm);     ///< Delete file
	static bool      file_rename(const Value::ArrayInstance &args, VM *vm);     ///< Rename file
	static Value     file_current_directory(const Value::ArrayInstance &args, VM *vm); ///< Get/set working directory
	static bool      file_copy(const Value::ArrayInstance &args, VM *vm);              ///< Copy file
	static bool      file_move(const Value::ArrayInstance &args, VM *vm);              ///< Move file
	static bool      file_property_edit(const Value::ArrayInstance &args, VM *vm);
	static i64       file_property_get(const Value::ArrayInstance &args, VM *vm);
	static bool      file_create(const Value::ArrayInstance &args, VM *vm);
	static Value     file_read_directory(const Value::ArrayInstance &args, VM *vm);
	static bool      file_create_directory(const Value::ArrayInstance &args, VM *vm);
	static bool      file_remove_directory(const Value::ArrayInstance &args, VM *vm);
	static PhsString file_join_path(const Value::ArrayInstance &args, VM *vm);
	static PhsString file_stem(const Value::ArrayInstance &args, VM *vm);         ///< Get the stem of a path
	static PhsString file_filename(const Value::ArrayInstance &args, VM *vm);     ///< Get the filename
	static PhsString file_extension(const Value::ArrayInstance &args, VM *vm);    ///< Get the extension of a path
	static bool      file_is_directory(const Value::ArrayInstance &args, VM *vm); ///< Check if path is directory
	static PhsString file_parent(const Value::ArrayInstance &args, VM *vm);       ///< Get the parent of a path
	static i64       file_get_size(const Value::ArrayInstance &args, VM *vm);
	static Value     file_memory_open(const Value::ArrayInstance &args, VM *vm); ///< Open an in-memory buffer as a stream
	static Value     file_pipe_open(const Value::ArrayInstance &args, VM *vm);   ///< Create a native pipe pair -> [readFd, writeFd]
	static i64       file_descriptor_kind(const Value::ArrayInstance &args, VM *vm);
#pragma endregion

#pragma region stdnet
#ifndef SANDBOXED
	static Value net_connect(const Value::ArrayInstance &args, VM *vm);       ///< (host, port, [timeoutMs]) -> fd | null
	static Value net_listen(const Value::ArrayInstance &args, VM *vm);        ///< (host, port, [backlog]) -> listener handle | null
	static Value net_accept(const Value::ArrayInstance &args, VM *vm);        ///< (listenerHandle, [timeoutMs]) -> {fd, host, port} | null
	static bool  net_close_listener(const Value::ArrayInstance &args, VM *vm);
	static bool  net_set_timeout(const Value::ArrayInstance &args, VM *vm);   ///< (fd, ms) -- send/recv timeout on a connected socket
	static bool  net_set_option(const Value::ArrayInstance &args, VM *vm);    ///< (fd, "nodelay"|"keepalive", value)
	static bool  net_shutdown(const Value::ArrayInstance &args, VM *vm);      ///< (fd, ["read"|"write"|"both"]) -- half-close
	static Value net_peer_address(const Value::ArrayInstance &args, VM *vm); ///< (fd) -> {host, port} | null
	static Value net_local_address(const Value::ArrayInstance &args, VM *vm);///< (fd) -> {host, port} | null
	static Value net_resolve(const Value::ArrayInstance &args, VM *vm);       ///< (host) -> addresses: string[] | null

	static Value net_udp_open(const Value::ArrayInstance &args, VM *vm);      ///< ([bindHost, bindPort]) -> handle | null
	static i64   net_udp_send_to(const Value::ArrayInstance &args, VM *vm);   ///< (handle, host, port, data) -> bytes sent
	static Value net_udp_recv_from(const Value::ArrayInstance &args, VM *vm);///< (handle, maxLen, [timeoutMs]) -> {data, host, port} | null
	static bool  net_udp_close(const Value::ArrayInstance &args, VM *vm);

#pragma endregion
#pragma region stdhttp
	static Value http_request(const Value::ArrayInstance &args, VM *vm); ///< (method, url, [body], [headers], [timeoutMs]) -> {status, headers, body}
#endif
#pragma endregion

#pragma region stdsys
	static i64   sys_get_free_memory(const Value::ArrayInstance &args, VM *vm); ///< Get current free memory
	static Value sys_wait_for_input(const Value::ArrayInstance &args, VM *vm);  ///< Wait for input
	static Value sys_shell(const Value::ArrayInstance &args, VM *vm);           ///< Run a shell command
	static Value sys_fork(const Value::ArrayInstance &args, VM *vm);          ///< blocking: {status, output|null}
	static Value sys_fork_detached(const Value::ArrayInstance &args, VM *vm); ///< non-blocking: {pid, handle, stdin, stdout, stderr}
	static i64   proc_wait(const Value::ArrayInstance &args, VM *vm);
	static Value proc_status(const Value::ArrayInstance &args, VM *vm);
	static bool  proc_kill(const Value::ArrayInstance &args, VM *vm);
	static bool  proc_forget(const Value::ArrayInstance &args, VM *vm); ///< fire-and-forget: free once it exits, no status/handle needed
	static bool  proc_free(const Value::ArrayInstance &args, VM *vm);   ///< blocks until exit, then frees
	static Value sys_error(const Value::ArrayInstance &args, VM *vm);           ///< Crash the VM / Program
	static Value sys_reset(const Value::ArrayInstance &args, VM *vm);           ///< Reset the VM
	static i64   sys_pid(const Value::ArrayInstance &args, VM *vm);             ///< Get the current process ID
	static i64   sys_os(const Value::ArrayInstance &args, VM *vm);              ///< Get the current OS
	static i64   sys_arch(const Value::ArrayInstance &args, VM *vm);
	static Value sys_isatty(const Value::ArrayInstance &args, VM *vm); ///< Check if the current output is a terminal
#endif
	static Value sys_env(const Value::ArrayInstance &args, VM *vm); ///< Get the current environment variables
	static Value sys_argv(const Value::ArrayInstance &args, VM *vm); ///< Get the current command line arguments -- deprecated, use sys_args() instead
	static i64   sys_argc(const Value::ArrayInstance &args, VM *vm); ///< Get the current number of command line arguments -- deprecated, use len(sys_args()) instead
	static Value sys_args(const Value::ArrayInstance &args, VM *vm); ///< Get args array
	static f64   sys_time(const Value::ArrayInstance &args, VM *vm);           ///< Current time
	static f64   sys_time_local(const Value::ArrayInstance &args, VM *vm);    ///< Current local time 
	static Value sys_time_formatted(const Value::ArrayInstance &args, VM *vm); ///< Current time formatted
	static Value sys_time_formatted_local(const Value::ArrayInstance &args, VM *vm); ///< Current local time formatted
	static Value sys_sleep(const Value::ArrayInstance &args, VM *vm);          ///< Sleep for a specified amount of time
	static Value sys_shutdown(const Value::ArrayInstance &args, VM *vm);       ///< Shutdown the VM
#pragma endregion

#pragma region stdini
	static Value     ini_read(const Value::ArrayInstance &args, VM *);
	static PhsString ini_write(const Value::ArrayInstance &args, VM *);
	static PhsString ini_read_entry(const Value::ArrayInstance &args, VM *);
	static PhsString ini_write_entry(const Value::ArrayInstance &args, VM *);
	static Value     ini_read_section(const Value::ArrayInstance &args, VM *);
	static PhsString ini_write_section(const Value::ArrayInstance &args, VM *);
	static bool      ini_has_section(const Value::ArrayInstance &args, VM *);
	static bool      ini_has_entry(const Value::ArrayInstance &args, VM *);
	static PhsString ini_remove_section(const Value::ArrayInstance &args, VM *);
	static PhsString ini_remove_entry(const Value::ArrayInstance &args, VM *);
	static bool      ini_empty(const Value::ArrayInstance &args, VM *);
#pragma endregion

#pragma region stdtype
	static i64         to_int(const Value::ArrayInstance &args, VM *vm);    ///< Convert to integer
	static f64         to_float(const Value::ArrayInstance &args, VM *vm);  ///< Convert to float
	static PhsString   to_string(const Value::ArrayInstance &args, VM *vm); ///< Convert to string
	static bool        to_bool(const Value::ArrayInstance &args, VM *vm);   ///< Convert to boolean
	static PhsString   to_json(const Value::ArrayInstance &args, VM *vm);   ///< Convert Value to JSON string
	static Value       from_json(const Value::ArrayInstance &args, VM *vm); ///< Convert JSON string to Value
	static PhsString   ascii_to_string(const Value::ArrayInstance &args, VM *vm); ///< Convert ascii to string
	static Value       get_struct_elements(const Value::ArrayInstance &args, VM *);
	static Value       get_struct_elements_values(const Value::ArrayInstance &args, VM *);
	static i64         get_type(const Value::ArrayInstance &args, VM *vm);
#pragma endregion

#pragma region stdarray
	static i64   array_length(const Value::ArrayInstance &args, VM *vm); ///< Get array length
	static Value array_push(const Value::ArrayInstance &args, VM *vm);   ///< Push to array
	static Value array_pop(const Value::ArrayInstance &args, VM *vm);    ///< Pop from array
	static Value array_peek(const Value::ArrayInstance &args, VM *vm);    ///< Peek from array
	static Value array_insert(const Value::ArrayInstance &args, VM *vm); ///< Insert into array
	static Value array_resize(const Value::ArrayInstance &args, VM *vm); ///< Resize array
	static Value array_join(const Value::ArrayInstance &args, VM *vm);
	static Value array_find(const Value::ArrayInstance &args, VM *vm);
#pragma endregion

#pragma region stdobject
	static Value object_has(const Value::ArrayInstance &args, VM *); ///< Check if struct has item
	static Value object_find(const Value::ArrayInstance &args, VM *); ///< Check if array has struct
	static Value object_filter(const Value::ArrayInstance &args, VM *); ///< Check if array has struct(s)
#pragma endregion

#pragma region stdrand
	static Value rand_seed(const Value::ArrayInstance &args, VM *vm);       ///< Seed the random number generator
	static i64   rand_next_range(const Value::ArrayInstance &args, VM *vm); ///< Get a random number in range
	static f64   rand_next_float(const Value::ArrayInstance &args,
	                               VM *vm); ///< Get a random float
	static Value rand_get_crypto_int(const Value::ArrayInstance &args, VM *);
	static Value rand_get_crypto_float(const Value::ArrayInstance &args, VM *);
#pragma endregion

#pragma region stdstr
	static i64       str_find(const Value::ArrayInstance &args, VM *vm);        ///< Find string in string
	static i64       str_len(const Value::ArrayInstance &args, VM *vm);         ///< Get string length
	static Value     str_char_at(const Value::ArrayInstance &args, VM *vm);     ///< Get character at index
	static Value     str_substr(const Value::ArrayInstance &args, VM *vm);      ///< Get substring
	static PhsString str_concat(const Value::ArrayInstance &args, VM *vm);      ///< Concatenate strings
	static PhsString str_upper(const Value::ArrayInstance &args, VM *vm);       ///< Convert to uppercase
	static PhsString str_lower(const Value::ArrayInstance &args, VM *vm);       ///< Convert to lowercase
	static Value     str_starts_with(const Value::ArrayInstance &args, VM *vm); ///< Check if string starts with
	static Value     str_ends_with(const Value::ArrayInstance &args, VM *vm);   ///< Check if string ends with
	static Value     str_split(const Value::ArrayInstance &args, VM *vm);      ///< Split string

	// StringBuilder functions
	static i64       sb_new(const Value::ArrayInstance &args, VM *vm);       ///< Create new string builder
	static i64       sb_prealloc(const Value::ArrayInstance &args, VM *vm); ///< Preallocate string builder
	static i64       sb_append(const Value::ArrayInstance &args, VM *vm);    ///< Append to string builder
	static PhsString sb_to_string(const Value::ArrayInstance &args, VM *vm); ///< Convert string builder to string
	static i64       sb_clear(const Value::ArrayInstance &args, VM *vm);     ///< Clear string builder
	static PhsString sb_free(const Value::ArrayInstance &args, VM *vm);      ///< Free string builder
#pragma endregion

#pragma region stdio
	static PhsString io_c_format(const Value::ArrayInstance &args, VM *vm); ///< Format string
#ifndef SANDBOXED
	static Value     io_clear(const Value::ArrayInstance &args, VM *vm); ///< Clear the console
#endif
	static PhsString io_printf(const Value::ArrayInstance &args, VM *vm); ///< Print formatted string
	static PhsString io_putf(const Value::ArrayInstance &args, VM *vm);   ///< Print formatted string with newline
#ifndef SANDBOXED
	static Value     io_gets(const Value::ArrayInstance &args, VM *vm); ///< Get line of stdin
	static Value     io_get_input(const Value::ArrayInstance &args, VM *vm); ///< Get stdin until EOF
#endif
	static PhsString io_putf_error(const Value::ArrayInstance &args,
	                                 VM *vm); ///< Print formatted string with newline to error output
	static PhsString io_print_error(const Value::ArrayInstance &args,
	                                 VM                       *vm);
#pragma endregion
};

} // namespace Phasor
