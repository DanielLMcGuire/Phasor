param(
    [string]$OldVersion = $args[0],
    [string]$NewVersion = $args[1]
)

if (-not $OldVersion -or -not $NewVersion) {
    Write-Host "Usage: .\Update-ManVersion.ps1 <old version> <new version>"
    exit
}

Get-ChildItem .\man -Recurse | Where-Object { $_.Extension -match '\.1$|\.3$|\.5$|\.7$' } | ForEach-Object {
    (Get-Content $_.FullName) -replace [regex]::Escape($OldVersion), $NewVersion | Set-Content $_.FullName
}

Write-Host "Updated all man pages from $OldVersion to $NewVersion"
