if (-not ("PHASOR_INTERNAL_ABI_3_1_1" -as [type])) {
    Add-Type @"
using System;
using System.Runtime.InteropServices;

public static class PHASOR_INTERNAL_ABI_3_1_1 {
    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int evaluatePHS(IntPtr vm, string script, string moduleName, string modulePath, bool verbose);
    [DllImport("phasorrt.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern int evaluatePUL(IntPtr vm, string script, string moduleName);
}
"@
}

function Start-PhasorEval {
    param(
        [Parameter(Mandatory=$true, Position=0, ValueFromPipeline=$true)]
        [string]$Script,

        [Parameter(Position=1)]
        [string]$ModuleName = "PowerShell (Start-PhasorEval)",

        [Parameter(Position=2)]
        [string]$ModulePath = "./",

        [Parameter(Position=3)]
        [bool]$EnableVerbose = $false
    )

    $return = [PHASOR_INTERNAL_ABI_3_1_1]::evaluatePHS([IntPtr]::Zero, $Script, $ModuleName, $ModulePath, $EnableVerbose)
    if ($return -ne 0) {
        Write-Error "Phasor script execution failed with code: $return"
    }
}

function Start-PulsarEval {
    param(
        [Parameter(Mandatory=$true, Position=0, ValueFromPipeline=$true)]
        [string]$Script,

        [Parameter(Position=1)]
        [string]$ModuleName = "PowerShell (Start-PulsarEval)"
    )

    $return = [PHASOR_INTERNAL_ABI_3_1_1]::evaluatePUL([IntPtr]::Zero, $Script, $ModuleName)
    if ($return -ne 0) {
        Write-Error "Pulsar script execution failed with code: $return"
    }
}

function Start-PhasorScript {
    param(
        [Parameter(Mandatory=$true, Position=0, ValueFromPipeline=$true)]
        [string]$ScriptPath,

        [Parameter(Position=1)]
        [string]$ModuleName = "PowerShell (Start-PhasorScript)",

        [Parameter(Position=2)]
        [bool]$EnableVerbose = $false
    )
    $content = Get-Content -Path $ScriptPath -Raw

    $scriptParentPath = Split-Path -Path $ScriptPath -Parent

    Start-PhasorEval $content $ModuleName $scriptParentPath $EnableVerbose
}

function Start-PulsarScript {
    param(
        [Parameter(Mandatory=$true, Position=0, ValueFromPipeline=$true)]
        [string]$ScriptPath,

        [Parameter(Position=1)]
        [string]$ModuleName = "PowerShell (Start-PulsarScript)"
    )
    $content = Get-Content -Path $ScriptPath -Raw

    Start-PulsarEval $content $ModuleName
}

function Start-PhasorRepl {
    phasorrepl
}
