if (-not ("PHASOR_INTERNAL_API_DO_NOT_USE" -as [type])) {
    Add-Type @"
using System;
using System.Runtime.InteropServices;

public static class PHASOR_INTERNAL_ABI_3_1_1 {
    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)] // in path
    public static extern void jitExec(string script, string moduleName, IntPtr nativeFunctionsVector);
}
"@
}

function Start-PhasorEval {
    param(
        [Parameter(Mandatory=$true, Position=0)]
        [string]$Script,

        [Parameter(Position=1)]
        [string]$ModuleName = "PowerShell (Start-PhasorEval)"
    )

    [PHASOR_INTERNAL_ABI_3_1_1]::jitExec($Script, $ModuleName, [IntPtr]::Zero)
}

function Start-PhasorScript {
    param(
        [Parameter(Mandatory=$true, Position=0)]
        [string]$ScriptPath,

        [Parameter(Position=1)]
        [string]$ModuleName = "PowerShell (Start-PhasorScript)"
    )
    $content = Get-Content -Path $ScriptPath -Raw

    Start-PhasorEval $content $ModuleName
}

function Start-PhasorRepl {
    phasor
}