#define PHASOR_FFI_BUILD_DLL
#include <PhasorFFI.h>

#include <iostream>

/**
 * @brief A simple native function to be exposed to the Phasor VM.
 *
 * This function demonstrates how to interact with the FFI:
 * - It accepts an array of PhasorValue arguments.
 * - It uses the `phasor_is_*` and `phasor_to_*` helpers to inspect arguments.
 * - It returns a PhasorValue using a `phasor_make_*` helper.
 */
PhasorValue hello_from_ffi(PhasorVM *vm, int argc, const PhasorValue *argv)
{
	std::cout << "Hello from a C++ FFI plugin!" << std::endl;
	std::cout << "Received " << argc << " arguments." << std::endl;

	for (int i = 0; i < argc; ++i)
	{
		std::cout << "  Arg " << i << ": ";
		switch (argv[i].type)
		{
		case PHASOR_TYPE_NULL:
			std::cout << "null";
			break;
		case PHASOR_TYPE_BOOL:
			std::cout << (phasor_to_bool(argv[i]) ? "true" : "false");
			break;
		case PHASOR_TYPE_INT:
			std::cout << phasor_to_int(argv[i]);
			break;
		case PHASOR_TYPE_FLOAT:
			std::cout << phasor_to_float(argv[i]);
			break;
		case PHASOR_TYPE_STRING: {
			std::cout << "\"" << phasor_to_string(argv[i]) << "\"";
			break;
		}
		}
		std::cout << std::endl;
	}

	// Return a string value back to the VM.
	// Note: For literal strings, this is safe. For dynamically allocated
	// strings, you must ensure the memory outlives the return to the VM.
	// The VM is expected to copy the string content.
	return phasor_make_string("Success from FFI!");
}

/**
 * @brief The required entry point for the plugin.
 *
 * The Phasor host calls this function after loading the shared library.
 * This function is responsible for registering all of the plugin's
 * native functions with the Phasor VM.
 */
void phasor_plugin_entry(const PhasorAPI *api, PhasorVM *vm)
{
	// Use the `register_function` pointer provided by the host to
	// register our `hello_from_ffi` function.
	api->register_function(vm, "hello", &hello_from_ffi);
}
