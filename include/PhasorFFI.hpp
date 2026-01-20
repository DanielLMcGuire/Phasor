#ifndef PHASOR_FFI_HPP
#define PHASOR_FFI_HPP

#include <cstdint>
#include <cstddef>

/*
* Phasor Foreign Function Interface
*
* This header provides the necessary definitions for creating third-party
* plugins for the Phasor scripting engine. It is designed to be a stable,
* C-compatible interface.
*
* USAGE:
* 1. Include this header in your C or C++ source file.
* 2. Implement the entry point function:
*    void phasor_plugin_entry(const PhasorAPI* api, PhasorVM* vm);
* 3. Inside this function, use the provided `api` object to register
*    your own native functions.
* 4. Compile your code as a shared library (.dll, .so, .dylib).
*/

#if defined(_WIN32) && defined(PHASOR_FFI_BUILD_DLL)
    #define PHASOR_FFI_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
    #define PHASOR_FFI_EXPORT __attribute__((visibility("default")))
#else
    #define PHASOR_FFI_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Opaque pointer to the Phasor Virtual Machine.
typedef struct PhasorVM PhasorVM;

// An enumeration of possible types a PhasorValue can hold.
typedef enum {
    PHASOR_TYPE_NULL,
    PHASOR_TYPE_BOOL,
    PHASOR_TYPE_INT,
    PHASOR_TYPE_FLOAT,
    PHASOR_TYPE_STRING,
    // Note: Structs are not directly supported in this version of the FFI
    // for simplicity, but can be added in the future.
} PhasorValueType;

// Represents a value in the Phasor VM.
// It's a tagged union holding one of several possible types.
typedef struct {
    PhasorValueType type;
    union {
        bool     b;
        int64_t  i;
        double   f;
        const char* s; // Note: For strings returned from the VM, this is valid.
                       // For strings passed to the VM, the VM makes a copy.
    } as;
} PhasorValue;

// -----------------------------------------------------------------------------
// Value Manipulation Functions (static inline for self-containment)
// -----------------------------------------------------------------------------

// Helper functions to create PhasorValue instances.
static inline PhasorValue phasor_make_null() {
    PhasorValue val;
    val.type = PHASOR_TYPE_NULL;
    return val;
}

static inline PhasorValue phasor_make_bool(bool b) {
    PhasorValue val;
    val.type = PHASOR_TYPE_BOOL;
    val.as.b = b;
    return val;
}

static inline PhasorValue phasor_make_int(int64_t i) {
    PhasorValue val;
    val.type = PHASOR_TYPE_INT;
    val.as.i = i;
    return val;
}

static inline PhasorValue phasor_make_float(double f) {
    PhasorValue val;
    val.type = PHASOR_TYPE_FLOAT;
    val.as.f = f;
    return val;
}

static inline PhasorValue phasor_make_string(const char* s) {
    PhasorValue val;
    val.type = PHASOR_TYPE_STRING;
    val.as.s = s;
    return val;
}

// Helper functions to check the type of a PhasorValue.
static inline bool phasor_is_null(PhasorValue val) { return val.type == PHASOR_TYPE_NULL; }
static inline bool phasor_is_bool(PhasorValue val) { return val.type == PHASOR_TYPE_BOOL; }
static inline bool phasor_is_int(PhasorValue val) { return val.type == PHASOR_TYPE_INT; }
static inline bool phasor_is_float(PhasorValue val) { return val.type == PHASOR_TYPE_FLOAT; }
static inline bool phasor_is_string(PhasorValue val) { return val.type == PHASOR_TYPE_STRING; }
static inline bool phasor_is_number(PhasorValue val) { return phasor_is_int(val) || phasor_is_float(val); }

// Helper functions to get the underlying value from a PhasorValue.
// Note: These do not perform type checking. Use the `phasor_is_*` functions first.
static inline bool phasor_to_bool(PhasorValue val) { return val.as.b; }
static inline int64_t phasor_to_int(PhasorValue val) { return val.as.i; }
static inline double phasor_to_float(PhasorValue val) {
    if (phasor_is_int(val)) return (double)val.as.i;
    return val.as.f;
}
static inline const char* phasor_to_string(PhasorValue val) { return val.as.s; }


// -----------------------------------------------------------------------------
// FFI API Definitions
// -----------------------------------------------------------------------------

// Signature for a native C function that can be registered with the Phasor VM.
typedef PhasorValue (*PhasorNativeFunction)(PhasorVM* vm, int argc, const PhasorValue* argv);

// Function pointer type for the function that registers a native function with the VM.
typedef void (*PhasorRegisterFunction)(PhasorVM* vm, const char* name, PhasorNativeFunction func);


// The collection of API functions that the Phasor host provides to the plugin.
typedef struct {
    // Registers a native C function with the given name.
    PhasorRegisterFunction register_function;
} PhasorAPI;


// -----------------------------------------------------------------------------
// Plugin Entry Point
// -----------------------------------------------------------------------------

/**
 * @brief The one and only entry point for a Phasor plugin.
 *
 * A Phasor-compatible shared library MUST export a C function with this exact
 * signature. The Phasor host will call this function when the plugin is loaded,
 * providing the plugin with a pointer to the host's API functions and the
 * current VM instance.
 *
 * @param api A pointer to a struct containing function pointers that the plugin
 *            can use to interact with the host (e.g., register functions).
 * @param vm  An opaque pointer to the VM instance. This should be passed back
 *            to any API functions that require it.
 */
PHASOR_FFI_EXPORT void phasor_plugin_entry(const PhasorAPI* api, PhasorVM* vm);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // PHASOR_FFI_HPP