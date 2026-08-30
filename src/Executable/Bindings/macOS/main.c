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

#define PHASOR_FFI_BUILD_DLL
#include <PhasorFFI.h>
#include <stdio.h>
#include "../../../Bindings/macOS/AppleScript.h"

static PhasorValue applescript_run(PhasorVM *, int argc, const PhasorValue *argv)
{
	int64_t status = -1;
	if (argc < 1 || !phasor_is_string(argv[0]))
	{
		return phasor_make_int(-1);
	}
	const char       *script = phasor_to_string(argv[0]);
	AppleScriptResult result = executeAppleScript(script);
	if (result.success)
	{
		if (result.output)
		{
			PhasorValue ret = phasor_make_string(result.output);
			freeAppleScriptResult(&result);
			return ret;
		}

		freeAppleScriptResult(&result);
		return phasor_make_int(0);
	} else {
		if (result.error)
		{
			puts(result.error);
		}
		PhasorValue ret = phasor_make_int(result.errorCode);
		freeAppleScriptResult(&result);
		return ret;
	}
	return phasor_make_int(status);
}

PHASOR_FFI_EXPORT void phasor_plugin_entry(const PhasorAPI *api, PhasorVM *vm)
{
	api->register_function(vm, "applescript_run", applescript_run);
}
