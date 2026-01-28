param(
    [string]$OldVersion = $args[0],
    [string]$NewVersion = $args[1]
)

if (-not $OldVersion -or -not $NewVersion) {
    Write-Host "Usage: .\Update-ResourceVersion.ps1 <old version> <new version>"
    exit
}

function DotToComma($version) {
    return $version -replace '\.', ','
}

$OldVersionComma = DotToComma $OldVersion
$NewVersionComma = DotToComma $NewVersion

$OldVersionString = "$OldVersion.0"
$NewVersionString = "$NewVersion.0"

Get-ChildItem App -Recurse -Filter *.rc | ForEach-Object {
    $lines = Get-Content $_.FullName
    $updated = @()

    foreach ($line in $lines) {
        $newLine = $line

        if ($line.TrimStart().StartsWith("FILEVERSION") -and $line -match [regex]::Escape($OldVersionComma)) {
            $newLine = $line -replace [regex]::Escape($OldVersionComma), $NewVersionComma
        }
        elseif ($line.TrimStart().StartsWith("PRODUCTVERSION") -and $line -match [regex]::Escape($OldVersionComma)) {
            $newLine = $line -replace [regex]::Escape($OldVersionComma), $NewVersionComma
        }
        elseif ($line -match 'VALUE "FileVersion",') {
            $parts = $line.Split('"')
            if ($parts[3] -eq $OldVersionString) { $parts[3] = $NewVersionString }
            $newLine = ($parts -join '"')
        }
        elseif ($line -match 'VALUE "ProductVersion",') {
            $parts = $line.Split('"')
            if ($parts[3] -eq $OldVersionString) { $parts[3] = $NewVersionString }
            $newLine = ($parts -join '"')
        }

        $updated += $newLine
    }

    $updated | Set-Content $_.FullName
}

Write-Host "Updated .rc files from $OldVersion to $NewVersion"