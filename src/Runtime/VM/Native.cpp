#ifndef CMAKE_PCH
#include <utility>

#include "VM.hpp"
#endif

#ifdef TRACING
#ifdef PHASOR_USES_BOOST
	#include <boost/assert/source_location.hpp>
	#define PHS_SRC_LOC() (std::format("{} @ {}:{}", BOOST_CURRENT_LOCATION.function_name(), BOOST_CURRENT_LOCATION.file_name(), BOOST_CURRENT_LOCATION.line()))
#else
	#define PHS_SRC_LOC() (std::format("VM::{}()", __func__))
#endif
#endif

void Phasor::VM::registerNativeFunction(const PhsString &name, NativeFunction fn)
{
#ifdef TRACING
	log(std::format("({})(\"{}\")\n", PHS_SRC_LOC(), name));
	flush();
#endif
	nativeFunctions[name] = std::move(fn);
}