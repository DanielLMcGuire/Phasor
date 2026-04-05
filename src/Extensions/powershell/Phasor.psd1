@{
    ModuleVersion     = '1.0.0'
    GUID              = 'b128b4a9-d5e1-4611-8f84-5ebf5b02ea18'
    RootModule        = 'Phasor.psm1'

    Author            = 'Daniel McGuire'
    CompanyName       = ''
    Copyright         = 'Daniel McGuire'
    Description       = 'PowerShell bindings for Phasor Runtime.'

    PowerShellVersion = '5.1'

    FunctionsToExport = @(
        'Start-PhasorEval',
        'Start-PulsarEval',
        'Start-PhasorScript',
        'Start-PulsarScript'
    )
    CmdletsToExport   = @()
    VariablesToExport = @()
    AliasesToExport   = @()

    PrivateData = @{
        PSData = @{
            Tags       = @('Phasor', 'Pulsar', 'Scripting', 'Runtime')
            LicenseUri   = 'https://github.com/DanielLMcGuire/Phasor/blob/master/LICENSE'
            ProjectUri   = 'https://github.com/DanielLMcGuire/Phasor'
        }
    }
}
