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
"@
}


$script:_PhasorTrackedStates = [System.Collections.Generic.List[PSCustomObject]]::new()

Register-EngineEvent -SourceIdentifier PowerShell.Exiting -Action {
    foreach ($state in $script:_PhasorTrackedStates) {
        if ($state.Pointer -ne [IntPtr]::Zero) {
            [PHASOR_INTERNAL_ABI_3_1_1]::freeState($state.Pointer) | Out-Null
            $state.Pointer = [IntPtr]::Zero
        }
    }
    $script:_PhasorTrackedStates.Clear()
} | Out-Null

<#
.SYNOPSIS
    Creates a new persistent Phasor VM state.

.DESCRIPTION
    Allocates a new VM state via phasorrt.dll and returns it as a PSCustomObject.
    Pass the returned object to any Start-Phasor* or Start-Pulsar* command via -State
    to reuse the same VM across multiple invocations, preserving globals and functions
    between calls.

    Always pair with Remove-PhasorState when the state is no longer needed to avoid
    leaking native memory.

.EXAMPLE
    $vm = New-PhasorState
    Start-PhasorEval -State $vm -Script 'let x = 42'
    Start-PhasorEval -State $vm -Script 'print(x)'   # x is still in scope
    Remove-PhasorState -State $vm
#>
function New-PhasorState {
    [CmdletBinding()]
    [OutputType([PSCustomObject])]
    param()

    $ptr = [PHASOR_INTERNAL_ABI_3_1_1]::createState()
    if ($ptr -eq [IntPtr]::Zero) {
        throw "phasorrt.dll returned a null pointer from createState()."
    }

    $state = [PSCustomObject]@{
        PSTypeName = 'PhasorState'
        Pointer    = $ptr
    }
    $script:_PhasorTrackedStates.Add($state)
    $state
}

<#
.SYNOPSIS
    Frees a Phasor VM state created by New-PhasorState.

.DESCRIPTION
    Releases the native memory held by the given state. After calling this function
    the state object must not be used again.

.PARAMETER State
    The PhasorState object returned by New-PhasorState.

.EXAMPLE
    Remove-PhasorState -State $vm
#>
function Remove-PhasorState {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [ValidateScript({ $_.PSTypeNames -contains 'PhasorState' })]
        [PSCustomObject]$State
    )

    process {
        $ok = [PHASOR_INTERNAL_ABI_3_1_1]::freeState($State.Pointer)
        if (-not $ok) {
            Write-Warning "freeState() returned false. The pointer may have already been freed."
        }
        # Null out the pointer so accidental reuse fails loudly.
        $State.Pointer = [IntPtr]::Zero
        # Deregister from the auto-cleanup list — state is already freed.
        $script:_PhasorTrackedStates.Remove($State) | Out-Null
    }
}

<#
.SYNOPSIS
    Resets a Phasor VM state.

.DESCRIPTION
    Always clears the stack, resets the program counter, and clears bytecode.
    Optionally resets registered functions and/or global variables.

.PARAMETER State
    The PhasorState object to reset.

.PARAMETER ResetFunctions
    When specified, clears all functions registered in the state.

.PARAMETER ResetVariables
    When specified, clears all global variables in the state.

.EXAMPLE
    # Soft reset — keep globals and functions, just reset execution state.
    Reset-PhasorState -State $vm

.EXAMPLE
    # Full reset — clear everything.
    Reset-PhasorState -State $vm -ResetFunctions -ResetVariables
#>
function Reset-PhasorState {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [ValidateScript({ $_.PSTypeNames -contains 'PhasorState' })]
        [PSCustomObject]$State,

        [Parameter()]
        [switch]$ResetFunctions,

        [Parameter()]
        [switch]$ResetVariables
    )

    process {
        $ok = [PHASOR_INTERNAL_ABI_3_1_1]::resetState($State.Pointer, $ResetFunctions.IsPresent, $ResetVariables.IsPresent)
        if (-not $ok) {
            Write-Error "resetState() failed for the provided state."
        }
    }
}

<#
.SYNOPSIS
    Register the Phasor Standard library into a Phasor VM state.

.DESCRIPTION
    Registers the Phasor Standard library functions into the Phasor VM.

.EXAMPLE
    Register-PhasorStdLib -State $vm
#>
function Register-PhasorStdLib {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [ValidateScript({ $_.PSTypeNames -contains 'PhasorState' })]
        [PSCustomObject]$State
    )

    [PHASOR_INTERNAL_ABI_3_1_1]::initStdLib($State.Pointer)
}

<#
.SYNOPSIS
    Compiles a Phasor (.phs) script string to bytecode.

.DESCRIPTION
    Returns a byte[] containing Phasor VM bytecode that can be passed to
    Invoke-PhasorBytecode for repeated execution without recompiling.

.PARAMETER Script
    The Phasor source code to compile.

.PARAMETER ModuleName
    A display name used in error reporting. Defaults to "PowerShell (Build-PhasorScript)".

.PARAMETER ModulePath
    Base path used to resolve compile-time imports. Defaults to "./".

.EXAMPLE
    $bytecode = Build-PhasorScript -Script 'print("hello")'
    Invoke-PhasorBytecode -Bytecode $bytecode
#>
function Build-PhasorScript {
    [CmdletBinding()]
    [OutputType([byte[]])]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [string]$Script,

        [Parameter(Position = 1)]
        [string]$ModuleName = "PowerShell (Build-PhasorScript)",

        [Parameter(Position = 2)]
        [string]$ModulePath = "./"
    )

    process {
        # First pass: calculate required buffer size.
        $outSize = [UIntPtr]::Zero
        $sizeOk = [PHASOR_INTERNAL_ABI_3_1_1]::compilePHS($Script, $ModuleName, $ModulePath, $null, [UIntPtr]::Zero, [ref]$outSize)
        if (-not $sizeOk) {
            Write-Error "Phasor compilation failed (size probe)."
            return
        }

        # Second pass: compile into a buffer of the known size.
        $buffer = New-Object byte[] $outSize.ToUInt64()
        $actualSize = [UIntPtr]::Zero
        $compileOk = [PHASOR_INTERNAL_ABI_3_1_1]::compilePHS($Script, $ModuleName, $ModulePath, $buffer, $outSize, [ref]$actualSize)
        if (-not $compileOk) {
            Write-Error "Phasor compilation failed (write pass)."
            return
        }

        # Trim to actual size in case it differs.
        if ($actualSize.ToUInt64() -ne $buffer.Length) {
            $trimmed = New-Object byte[] $actualSize.ToUInt64()
            [Array]::Copy($buffer, $trimmed, $actualSize.ToUInt64())
            $trimmed
        } else {
            $buffer
        }
    }
}

<#
.SYNOPSIS
    Compiles a Pulsar (.pul) script string to bytecode.

.DESCRIPTION
    Returns a byte[] containing Phasor VM bytecode compiled from Pulsar source.

.PARAMETER Script
    The Pulsar source code to compile.

.PARAMETER ModuleName
    A display name used in error reporting. Defaults to "PowerShell (Build-PulsarScript)".

.EXAMPLE
    $bytecode = Build-PulsarScript -Script 'print("hello")'
    Invoke-PhasorBytecode -Bytecode $bytecode
#>
function Build-PulsarScript {
    [CmdletBinding()]
    [OutputType([byte[]])]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [string]$Script,

        [Parameter(Position = 1)]
        [string]$ModuleName = "PowerShell (Build-PulsarScript)"
    )

    process {
        $outSize = [UIntPtr]::Zero
        $sizeOk = [PHASOR_INTERNAL_ABI_3_1_1]::compilePUL($Script, $ModuleName, $null, [UIntPtr]::Zero, [ref]$outSize)
        if (-not $sizeOk) {
            Write-Error "Pulsar compilation failed (size probe)."
            return
        }

        $buffer = New-Object byte[] $outSize.ToUInt64()
        $actualSize = [UIntPtr]::Zero
        $compileOk = [PHASOR_INTERNAL_ABI_3_1_1]::compilePUL($Script, $ModuleName, $buffer, $outSize, [ref]$actualSize)
        if (-not $compileOk) {
            Write-Error "Pulsar compilation failed (write pass)."
            return
        }

        if ($actualSize.ToUInt64() -ne $buffer.Length) {
            $trimmed = New-Object byte[] $actualSize.ToUInt64()
            [Array]::Copy($buffer, $trimmed, $actualSize.ToUInt64())
            $trimmed
        } else {
            $buffer
        }
    }
}

<#
.SYNOPSIS
    Executes pre-compiled Phasor VM bytecode.

.DESCRIPTION
    Runs a byte[] produced by Build-PhasorScript or Build-PulsarScript.
    Accepts an optional persistent State and argument list forwarded to the script.

.PARAMETER Bytecode
    The compiled bytecode byte array to execute.

.PARAMETER State
    An optional PhasorState to execute within. If omitted, a transient state is used.

.PARAMETER ModuleName
    A display name used in error reporting. Defaults to "PowerShell (Invoke-PhasorBytecode)".

.PARAMETER ArgumentList
    An optional array of string arguments passed to the script as its argv.

.EXAMPLE
    $bc = Build-PhasorScript -Script 'print("hello")'
    Invoke-PhasorBytecode -Bytecode $bc

.EXAMPLE
    $bc = Build-PhasorScript -Script 'print(args[0])'
    Invoke-PhasorBytecode -Bytecode $bc -ArgumentList "world"
#>
function Invoke-PhasorBytecode {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [byte[]]$Bytecode,

        [Parameter()]
        [ValidateScript({ $_.PSTypeNames -contains 'PhasorState' })]
        [PSCustomObject]$State,

        [Parameter()]
        [string]$ModuleName = "PowerShell (Invoke-PhasorBytecode)",

        [Parameter()]
        [string[]]$ArgumentList = @()
    )

    process {
        $ptr   = if ($State) { $State.Pointer } else { [IntPtr]::Zero }
        $argc  = $ArgumentList.Count
        $argv  = if ($argc -gt 0) { $ArgumentList } else { $null }

        $return = [PHASOR_INTERNAL_ABI_3_1_1]::exec(
            $ptr, $Bytecode, [UIntPtr][uint64]$Bytecode.Length, $ModuleName, $argc, $argv
        )
        if ($return -ne 0) {
            Write-Error "Phasor bytecode execution failed with exit code: $return"
        }
    }
}

<#
.SYNOPSIS
    Evaluates a Phasor (.phs) script string.

.DESCRIPTION
    Compiles and executes a Phasor script string via phasorrt.dll.
    Pass a persistent -State to share VM context across multiple calls.

.PARAMETER Script
    The Phasor script content to evaluate.

.PARAMETER State
    An optional PhasorState to execute within. If omitted, a transient state is used.

.PARAMETER ModuleName
    A display name used in error reporting. Defaults to "PowerShell (Start-PhasorEval)".

.PARAMETER ModulePath
    Base path used to resolve compile-time imports. Defaults to "./".

.PARAMETER EnableVerbose
    When set to $true, prints the AST to stdout.

.EXAMPLE
    Start-PhasorEval -Script 'print("hello")'

.EXAMPLE
    $vm = New-PhasorState
    Start-PhasorEval -State $vm -Script 'let x = 10'
    Start-PhasorEval -State $vm -Script 'print(x)'
    Remove-PhasorState $vm

.EXAMPLE
    'print("hello")' | Start-PhasorEval
#>
function Start-PhasorEval {
    [CmdletBinding()]
    param(
        [Parameter(Mandatory = $true, Position = 0, ValueFromPipeline = $true)]
        [string]$Script,

        [Parameter()]
        [ValidateScript({ $_.PSTypeNames -contains 'PhasorState' })]
        [PSCustomObject]$State,

        [Parameter()]
        [string]$ModuleName = "PowerShell (Start-PhasorEval)",

        [Parameter()]
        [string]$ModulePath = "./"
    )

    process {
        $ptr    = if ($State) { $State.Pointer } else { [IntPtr]::Zero }
        $return = [PHASOR_INTERNAL_ABI_3_1_1]::evaluatePHS($ptr, $Script, $ModuleName, $ModulePath, $VerbosePreference)
        if ($return -ne 0) {
            Write-Error "Phasor script execution failed with exit code: $return"
        }
    }
}

<#
.SYNOPSIS
    Evaluates a Pulsar (.pul) script string.

.DESCRIPTION
    Compiles and executes a Pulsar script string via phasorrt.dll.
    Pass a persistent -State to share VM context across multiple calls.

.PARAMETER Script
    The Pulsar script content to evaluate.

.PARAMETER State
    An optional PhasorState to execute within. If omitted, a transient state is used.

.PARAMETER ModuleName
    A display name used in error reporting. Defaults to "PowerShell (Start-PulsarEval)".

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

        [Parameter()]
        [ValidateScript({ $_.PSTypeNames -contains 'PhasorState' })]
        [PSCustomObject]$State,

        [Parameter()]
        [string]$ModuleName = "PowerShell (Start-PulsarEval)"
    )

    process {
        $ptr    = if ($State) { $State.Pointer } else { [IntPtr]::Zero }
        $return = [PHASOR_INTERNAL_ABI_3_1_1]::evaluatePUL($ptr, $Script, $ModuleName)
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

.PARAMETER State
    An optional PhasorState to execute within. If omitted, a transient state is used.

.PARAMETER ModuleName
    A display name used in error reporting. Defaults to "PowerShell (Start-PhasorScript)".

.PARAMETER EnableVerbose
    When set to $true, prints the AST to stdout.

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

        [Parameter()]
        [ValidateScript({ $_.PSTypeNames -contains 'PhasorState' })]
        [PSCustomObject]$State,

        [Parameter()]
        [string]$ModuleName = "PowerShell (Start-PhasorScript)"
    )

    process {
        $content          = Get-Content -Path $ScriptPath -Raw
        $scriptParentPath = Split-Path -Path $ScriptPath -Parent
        $params = @{
            Script        = $content
            ModuleName    = $ModuleName
            ModulePath    = $scriptParentPath
        }
        if ($State) { $params['State'] = $State }
        Start-PhasorEval @params
    }
}

<#
.SYNOPSIS
    Runs a Pulsar (.pul) script file.

.DESCRIPTION
    Reads the specified file and passes its contents to Start-PulsarEval.

.PARAMETER ScriptPath
    Path to the .pul script file to execute.

.PARAMETER State
    An optional PhasorState to execute within. If omitted, a transient state is used.

.PARAMETER ModuleName
    A display name used in error reporting. Defaults to "PowerShell (Start-PulsarScript)".

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

        [Parameter()]
        [ValidateScript({ $_.PSTypeNames -contains 'PhasorState' })]
        [PSCustomObject]$State,

        [Parameter()]
        [string]$ModuleName = "PowerShell (Start-PulsarScript)"
    )

    process {
        $content = Get-Content -Path $ScriptPath -Raw
        $params  = @{ Script = $content; ModuleName = $ModuleName }
        if ($State) { $params['State'] = $State }
        Start-PulsarEval @params
    }
}

Export-ModuleMember -Function @(
    # State lifecycle
    'New-PhasorState'
    'Remove-PhasorState'
    'Reset-PhasorState'
    'Register-PhasorStdLib'
    # Compilation
    'Build-PhasorScript'
    'Build-PulsarScript'
    # Bytecode execution
    'Invoke-PhasorBytecode'
    # Source evaluation
    'Start-PhasorEval'
    'Start-PulsarEval'
    'Start-PhasorScript'
    'Start-PulsarScript'
)