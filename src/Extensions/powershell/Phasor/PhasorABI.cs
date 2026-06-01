using System;
using System.Runtime.InteropServices;

public static class PHASOR_INTERNAL_ABI_4_0_0 {
    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int evaluatePHS(IntPtr vm, string script, string moduleName, string modulePath, bool verbose);

    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int evaluatePUL(IntPtr vm, string script, string moduleName);

    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int exec(IntPtr vm, byte[] bytecode, UIntPtr bytecodeSize, string moduleName, int argc, string[] argv);

    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr createState();

    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool freeState(IntPtr vm);

    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void initStdLib(IntPtr vm);

    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool resetState(IntPtr vm, bool resetFunctions, bool resetVariables);

    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool compilePHS(string script, string moduleName, string modulePath,
                                         byte[] buffer, UIntPtr bufferSize, out UIntPtr outSize);

    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool compilePUL(string script, string moduleName,
                                         byte[] buffer, UIntPtr bufferSize, out UIntPtr outSize);
}