# Copyright 2026 Daniel McGuire
# Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
# Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http:#www.apache.org/licenses/LICENSE-2.0
# or https:#phasor.pages.dev/LICENSE.txt
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

@{
    ModuleVersion     = '4.0.0'
    GUID              = 'b128b4a9-d5e1-4611-8f84-5ebf5b02ea18'
    RootModule        = 'Phasor.psm1'
    Author            = 'Daniel McGuire'
    CompanyName       = ''
    Copyright         = 'Daniel McGuire'
    Description       = 'PowerShell bindings for Phasor Runtime.'
    PowerShellVersion = '5.1'
    FunctionsToExport = @(
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
    CmdletsToExport   = @()
    VariablesToExport = @()
    AliasesToExport   = @()
    PrivateData = @{
        PSData = @{
            Tags         = @('Phasor', 'Pulsar', 'Scripting', 'Runtime')
            LicenseUri   = 'https://github.com/DanielLMcGuire/Phasor/blob/master/LICENSE'
            ProjectUri   = 'https://github.com/DanielLMcGuire/Phasor'
        }
    }
}