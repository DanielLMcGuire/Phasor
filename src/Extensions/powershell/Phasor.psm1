#Requires -Version 5.1

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

<#
.SYNOPSIS
    Evaluates a Phasor (.phs) script string.

.DESCRIPTION
    Passes a Phasor script string directly to the phasorrt.dll runtime for evaluation.

.PARAMETER Script
    The Phasor script content to evaluate.

.PARAMETER ModuleName
    A display name identifying the caller. Defaults to "PowerShell (Start-PhasorEval)".

.PARAMETER ModulePath
    The base path used to resolve relative imports within the script. Defaults to "./".

.PARAMETER EnableVerbose
    When set to $true, enables verbose output from the runtime.

.EXAMPLE
    Start-PhasorEval -Script 'print("hello")'

.EXAMPLE
    'print("hello")' | Start-PhasorEval
#>
function Start-PhasorEval {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [string]$Script,

        [Parameter(Position = 1)]
        [string]$ModuleName = "PowerShell (Start-PhasorEval)",

        [Parameter(Position = 2)]
        [string]$ModulePath = "./",

        [Parameter(Position = 3)]
        [bool]$EnableVerbose = $false
    )

    process {
        $return = [PHASOR_INTERNAL_ABI_3_1_1]::evaluatePHS(
            [IntPtr]::Zero, $Script, $ModuleName, $ModulePath, $EnableVerbose
        )
        if ($return -ne 0) {
            Write-Error "Phasor script execution failed with exit code: $return"
        }
    }
}

<#
.SYNOPSIS
    Evaluates a Pulsar (.pul) script string.

.DESCRIPTION
    Passes a Pulsar script string directly to the phasorrt.dll runtime for evaluation.

.PARAMETER Script
    The Pulsar script content to evaluate.

.PARAMETER ModuleName
    A display name identifying the caller. Defaults to "PowerShell (Start-PulsarEval)".

.EXAMPLE
    Start-PulsarEval -Script 'print("hello")'

.EXAMPLE
    'print("hello")' | Start-PulsarEval
#>
function Start-PulsarEval {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [string]$Script,

        [Parameter(Position = 1)]
        [string]$ModuleName = "PowerShell (Start-PulsarEval)"
    )

    process {
        $return = [PHASOR_INTERNAL_ABI_3_1_1]::evaluatePUL(
            [IntPtr]::Zero, $Script, $ModuleName
        )
        if ($return -ne 0) {
            Write-Error "Pulsar script execution failed with exit code: $return"
        }
    }
}

<#
.SYNOPSIS
    Runs a Phasor (.phs) script file.

.DESCRIPTION
    Reads the specified file and passes its contents to Start-PhasorEval.
    The script's parent directory is automatically used as the module path.

.PARAMETER ScriptPath
    Path to the .phs script file to execute.

.PARAMETER ModuleName
    A display name identifying the caller. Defaults to "PowerShell (Start-PhasorScript)".

.PARAMETER EnableVerbose
    When set to $true, enables verbose output from the runtime.

.EXAMPLE
    Start-PhasorScript -ScriptPath .\myscript.phs

.EXAMPLE
    .\myscript.phs | Start-PhasorScript
#>
function Start-PhasorScript {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [ValidateScript({ Test-Path $_ -PathType Leaf })]
        [string]$ScriptPath,

        [Parameter(Position = 1)]
        [string]$ModuleName = "PowerShell (Start-PhasorScript)",

        [Parameter(Position = 2)]
        [bool]$EnableVerbose = $false
    )

    process {
        $content = Get-Content -Path $ScriptPath -Raw
        $scriptParentPath = Split-Path -Path $ScriptPath -Parent
        Start-PhasorEval -Script $content -ModuleName $ModuleName -ModulePath $scriptParentPath -EnableVerbose $EnableVerbose
    }
}

<#
.SYNOPSIS
    Runs a Pulsar (.pul) script file.

.DESCRIPTION
    Reads the specified file and passes its contents to Start-PulsarEval.

.PARAMETER ScriptPath
    Path to the .pul script file to execute.

.PARAMETER ModuleName
    A display name identifying the caller. Defaults to "PowerShell (Start-PulsarScript)".

.EXAMPLE
    Start-PulsarScript -ScriptPath .\myscript.pul

.EXAMPLE
    .\myscript.pul | Start-PulsarScript
#>
function Start-PulsarScript {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [ValidateScript({ Test-Path $_ -PathType Leaf })]
        [string]$ScriptPath,

        [Parameter(Position = 1)]
        [string]$ModuleName = "PowerShell (Start-PulsarScript)"
    )

    process {
        $content = Get-Content -Path $ScriptPath -Raw
        Start-PulsarEval -Script $content -ModuleName $ModuleName
    }
}

Export-ModuleMember -Function @(
    'Start-PhasorEval',
    'Start-PulsarEval',
    'Start-PhasorScript',
    'Start-PulsarScript'
)
