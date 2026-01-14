#include "winuser.hpp"

Value winuser::registerFunctions(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 0, "win_registerFunctions");
    vm->registerNativeFunction("win_MessageBox", MessageBox_ours);
    return true;
}

// int MessageBoxA([in, optional] HWND hWnd, [in, optional] LPCSTR lpText, [in, optional] LPCSTR lpCaption, [in] UINT uType);
Value winuser::MessageBox_ours(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 2, "win_MessageBox", true);
    HWND hWnd = args[0].isNull() ? nullptr : win::asHWND(args[0]);
    
    std::string textStr = args[1].isNull() ? "" : args[1].asString();
    std::string captionStr = args[2].isNull() ? "" : args[2].asString();
    
    LPCSTR lpText = args[1].isNull() ? nullptr : textStr.c_str();
    LPCSTR lpCaption = args[2].isNull() ? nullptr : captionStr.c_str();
    UINT uType = win::asUINT(args[3]);
    int result = ::MessageBoxA(hWnd, lpText, lpCaption, uType);
    return result;
}