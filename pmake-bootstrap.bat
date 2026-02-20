@echo off
setlocal
echo.
set SCRIPT_DIR=%~dp0
set TMP_DIR=%SCRIPT_DIR%.tmp

if "%1"=="" (
    echo Usage: %0 [32^|64^|arm64]
    echo.
    echo   32     - Build for 32-bit x86
    echo   64     - Build for 64-bit x86-64
    echo   arm64  - Build for ARM64
    exit /b 1
)
set ARCH=%1
if /i "%ARCH%"=="32" (
    set CMAKE_PRESET=windows-32-rel
    set VSBUILD_ARCH=x86
    set VSBUILD_HOST_ARCH=x86
    set VSSTUDIO_COMPONENT=Microsoft.VisualStudio.Component.VC.Tools.x86.x64
    set PMAKE_TARGET=windows-32-rel
) else if /i "%ARCH%"=="64" (
    set CMAKE_PRESET=windows-32-rel
    set VSBUILD_ARCH=x86
    set VSBUILD_HOST_ARCH=x64
    set VSSTUDIO_COMPONENT=Microsoft.VisualStudio.Component.VC.Tools.x86.x64
    set PMAKE_TARGET=windows-64-rel
) else if /i "%ARCH%"=="arm64" (
    set CMAKE_PRESET=windows-arm64-rel
    set VSBUILD_ARCH=arm64
    set VSBUILD_HOST_ARCH=x64
    set VSSTUDIO_COMPONENT=Microsoft.VisualStudio.Component.VC.Tools.x86.x64
    set PMAKE_TARGET=windows-arm64-rel
) else (
    echo Invalid architecture: %ARCH%
    echo Valid options: 32, 64, arm64
    exit /b 1
)
echo SYNC
git submodule update
if errorlevel 1 (
    echo Failed to update git submodules
    exit /b 1
)
echo CFG vs-vswhere
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo vswhere.exe not found
    exit /b 1
)
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS_PATH=%%i"
if not defined VS_PATH (
    echo No Visual Studio installation with required C++ tools found
    exit /b 1
)
echo CFG vs-vcvars
call "%VS_PATH%\Common7\Tools\VsDevCmd.bat" -arch=%VSBUILD_ARCH% -host_arch=%VSBUILD_HOST_ARCH% -no_logo
if errorlevel 1 (
    echo Failed to initialize Visual Studio environment
    exit /b 1
)
echo CFG phasor-cmake
cmake -S "%SCRIPT_DIR:~0,-1%" -B "%TMP_DIR%" --preset %CMAKE_PRESET% 2>&1 1>nul
if errorlevel 1 (
    echo CMake configuration failed
    exit /b 1
)
echo CXX phasor_native_runtime_static
ninja -C "%TMP_DIR%" phasor_native_runtime_static 2>&1 1>nul
if errorlevel 1 (
    echo Building runtime failed
    exit /b 1
)
echo CXX phasor_cxx_transpiler
ninja -C "%TMP_DIR%" phasor_cxx_transpiler 2>&1 1>nul
if errorlevel 1 (
    echo Building compiler failed
    exit /b 1
)
echo PHS pmake.phs
ninja -C "%TMP_DIR%" pmake 2>&1 1>nul
if errorlevel 1 (
    echo Building pmake failed
    exit /b 1
)
echo INST pmake.exe .
copy "%TMP_DIR%\thirdparty\pmake\Executable\pmake\pmake.exe" "%SCRIPT_DIR%pmake.exe" /Y 2>&1 1>nul
if errorlevel 1 (
    echo Copying pmake.exe failed
    exit /b 1
)
echo CLEAN
rmdir /s /q "%TMP_DIR%"
if errorlevel 1 (
    echo Failed to remove temporary files
    exit /b 1
)
echo DONE
echo.
echo Setting up build...\""
pmake %PMAKE_TARGET% -f
if errorlevel 1 (
    echo pmake setup failed
    exit /b 1
)
echo Showing options...
pmake -h
endlocal