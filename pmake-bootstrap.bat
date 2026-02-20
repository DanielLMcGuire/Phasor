@echo off
setlocal
set SCRIPT_DIR=%~dp0
set TMP_DIR=%SCRIPT_DIR%.tmp
cmake -S %SCRIPT_DIR% -B %TMP_DIR% -DCMAKE_BUILD_TYPE=Release -G Ninja 2>&1 1>nul
echo Building...
ninja -C %TMP_DIR% pmake 2>&1 1>nul
echo Copying files...
copy %TMP_DIR%\src\Executable\utils\pmake\pmake.exe %SCRIPT_DIR%pmake.exe /Y 2>&1 1>nul
echo Cleaning up...
rmdir /s /q %TMP_DIR% 2>&1 1>nul
endlocal