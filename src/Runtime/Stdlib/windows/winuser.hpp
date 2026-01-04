#include "../../VM/VM.hpp"
#include "../../Value.hpp"
#include "../StdLib.hpp" // For argument checking
#include <cstdint>

#include <Windows.h>

#include "vhandle.hpp"   // For Virtual Handles
#include "winType.hpp"   // For Win32 Types

class winuser {
public:
    /// @brief Register Windows API User functions
    static Value registerFunctions(const std::vector<Value> &args, VM *vm);
private:
    /// @brief Show a message box
    static Value MessageBox_ours(const std::vector<Value> &args, VM *vm);
};