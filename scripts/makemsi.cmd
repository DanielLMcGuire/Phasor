@echo off
setlocal

set WIX_DIR=%~dp0.wix
set WIX_EXTENSIONS=%WIX_DIR%\extensions
set NUGET_EXE=%WIX_DIR%\nuget.exe
set HEAT_EXE=%WIX_DIR%\heat\WixToolset.Heat.4.0.5\tools\net472\x64\heat.exe

if not exist "%WIX_DIR%\wix.exe" (
    echo Installing WiX...
    dotnet tool install wix --version 4.0.5 --tool-path "%WIX_DIR%"
)

if not exist "%NUGET_EXE%" (
    echo Downloading nuget.exe...
    curl -L https://dist.nuget.org/win-x86-commandline/latest/nuget.exe -o "%NUGET_EXE%"
)

if not exist "%HEAT_EXE%" (
    echo Downloading heat.exe...
    "%NUGET_EXE%" install WixToolset.Heat -Version 4.0.5 -OutputDirectory "%WIX_DIR%\heat"
)

if not exist "%WIX_EXTENSIONS%" mkdir "%WIX_EXTENSIONS%"
"%WIX_DIR%\wix.exe" extension add WixToolset.UI.wixext/4.0.5

echo Harvesting files...
"%HEAT_EXE%" dir ..\out -cg PhasorFiles -dr INSTALLFOLDER -var var.SourceDir -ag -sfrag -sreg -srd -out harvest.wxs

echo Building MSI...
"%WIX_DIR%\wix.exe" build installer.wxs harvest.wxs -d SourceDir="..\out" -arch x64 -o phasor-4.0.0.msi -ext WixToolset.UI.wixext