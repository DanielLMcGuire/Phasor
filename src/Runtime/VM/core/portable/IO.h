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

#include <stdint.h>

// Phasor vmcore/portable IO

#ifdef __cplusplus
extern "C"
{
#endif
	/// @brief Native print function
	void c_print_stdout(const char *s, int64_t len);
	/// @brief Native print error function
	void c_print_stderr(const char *s, int64_t len);
	/// @brief CRT system call
	int64_t c_system(const char *cmd);
	/// @brief CRT system call, get out
	char *c_system_out(const char *cmd);
	/// @brief CRT system call, get err
	char *c_system_err(const char *cmd);
#ifdef __cplusplus
}
#endif
