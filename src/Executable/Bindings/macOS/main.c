#define PHASOR_FFI_BUILD_DLL
#include <PhasorFFI.h>
#include <stdio.h>
#include "../../../Bindings/macOS/AppleScript.h"

static PhasorValue applescript_run(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	int status = -1;
	if (argc < 1 || !phasor_is_string(argv[0]))
		return phasor_make_int(-1);
	const char *script = phasor_to_string(argv[0]);
	AppleScriptResult result = executeAppleScript(script);
	if (result.sucess) { 
		if (result.output)  {
			freeAppleScriptResult(result);
			PhasorValue ret = phasor_make_string(result.output);
			return ret;
		} else {
			freeAppleScriptResult(result);
			return phasor_make_int(0);
		}
	} else { 
		if(result.error) printf(result.error); 
		PhasorValue ret = phasor_make_int(result.errorCode);
		freeAppleScriptResult(result);
		return ret;
	}
}

PHASOR_FFI_EXPORT void phasor_plugin_entry(const PhasorAPI *api, PhasorVM *vm)
{
	api->register_function(vm, "applescript_run", applescript_run);
}
