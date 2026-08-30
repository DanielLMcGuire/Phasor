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