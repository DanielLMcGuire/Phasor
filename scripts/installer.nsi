!include "x64.nsh"
!define APP_NAME "Phasor Programming Language"
!define COMP_NAME "Phasor"
!define VERSION "3.1.0.0"
!define COPYRIGHT "(C) 2026 Daniel McGuire"
!define LICENSE_TXT "../LICENSE"
!define DESCRIPTION "Functional VM Compiled Programming Language"
!define INSTALLER_NAME "phasor-3.1.0_setup.exe"
!define INSTALL_TYPE "SetShellVarContext all"
!define REG_ROOT "HKLM"
!define MAIN_APP_EXE "bin\phasor.exe"
!define REG_APP_PATH "Software\Microsoft\Windows\CurrentVersion\App Paths\${MAIN_APP_EXE}"
!define UNINSTALL_PATH "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

######################################################################

VIProductVersion  "${VERSION}"
VIAddVersionKey "ProductName"  "${APP_NAME}"
VIAddVersionKey "CompanyName"  "${COMP_NAME}"
VIAddVersionKey "LegalCopyright"  "${COPYRIGHT}"
VIAddVersionKey "FileDescription"  "${DESCRIPTION}"
VIAddVersionKey "FileVersion"  "${VERSION}"

######################################################################

SetCompressor /SOLID /FINAL LZMA
Name "${APP_NAME}"
Caption "${APP_NAME}"
OutFile "${INSTALLER_NAME}"
BrandingText "${COMP_NAME}"
XPStyle on
InstallDirRegKey "${REG_ROOT}" "${UNINSTALL_PATH}" "InstallLocation"
InstallDir "$PROGRAMFILES64\Phasor Programming Language"


######################################################################

!include "MUI2.nsh"
!include "x64.nsh"

!define MUI_ABORTWARNING
!define MUI_UNABORTWARNING

!insertmacro MUI_PAGE_WELCOME

!ifdef LICENSE_TXT
!insertmacro MUI_PAGE_LICENSE "${LICENSE_TXT}"
!endif

!insertmacro MUI_PAGE_DIRECTORY

!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

######################################################################

Function .onInit
    ${If} ${RunningX64}
    ${Else}
        MessageBox MB_OK|MB_ICONEXCLAMATION "This program requires a 64-bit processor."
        Abort
    ${EndIf}
FunctionEnd

######################################################################

Section -MainProgram
${INSTALL_TYPE}
SetOverwrite ifnewer
SetOutPath "$INSTDIR"
file /r "..\out\"
file "..\assets\phasor.ico"


SectionEnd

######################################################################

Section -Icons_Reg
SetOutPath "$INSTDIR"

WriteUninstaller "$INSTDIR\uninstall.exe"

WriteRegStr ${REG_ROOT} "${REG_APP_PATH}" "" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayName" "${APP_NAME}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "InstallLocation" "$INSTDIR"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayIcon" "$INSTDIR\phasor.ico"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayVersion" "${VERSION}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "Publisher" "${COMP_NAME}"

SetRegView 64
ClearErrors

ReadRegDWORD $R0 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Installed"
SetRegView 32

IfErrors DownloadVC
IntCmp $R0 1 SkipDownload DownloadVC

SkipDownload:
  DetailPrint "Visual C++ Redistributable (x64) is already installed."
  Goto EndVC

DownloadVC:
  DetailPrint "Downloading Visual C++ Redistributable (x64)..."
  NSISdl::download "https://aka.ms/vs/17/release/vc_redist.x64.exe" "$PLUGINSDIR\vc_redist.x64.exe"
  Pop $R0
  StrCmp $R0 "success" 0 +3
    MessageBox MB_OK "Download failed: $R0"
    Abort

  DetailPrint "Installing Visual C++ Redistributable (x64)..."
  ExecWait '"$PLUGINSDIR\vc_redist.x64.exe" /install /passive /norestart /log "$PLUGINSDIR\vslog.txt"'
  StrCmp $R0 "0" 0 +2
    MessageBox MB_OK "VC++ Redistributable installation might have failed."

EndVC:

WriteRegStr HKCU "Software\Classes\.phir" "" "PhasorASM.PHIR"
WriteRegStr HKCU "Software\Classes\.phs"  "" "Phasor.PHS"
WriteRegStr HKCU "Software\Classes\.phsb" "" "Phasor.PHSB"
WriteRegStr HKCU "Software\Classes\.pul"  "" "Pulsar.PUL"


WriteRegStr HKCU "Software\Classes\Phasor.PHSB" "" "Phasor Binary File"
WriteRegStr HKCU "Software\Classes\Phasor.PHSB\DefaultIcon" "" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr HKCU "Software\Classes\Phasor.PHSB\shell\open" "FriendlyAppName" "Phasor"
WriteRegStr HKCU "Software\Classes\Phasor.PHSB\shell\open\command" "" '"$INSTDIR\${MAIN_APP_EXE}" "%1" %*'

WriteRegStr HKCU "Software\Classes\PhasorDecomp.PHSB" "" "Phasor Binary File"
WriteRegStr HKCU "Software\Classes\PhasorDecomp.PHSB\DefaultIcon" "" "$INSTDIR\bin\phasordecomp.exe"
WriteRegStr HKCU "Software\Classes\PhasorDecomp.PHSB\shell\open" "FriendlyAppName" "Phasor Decompiler"
WriteRegStr HKCU "Software\Classes\PhasorDecomp.PHSB\shell\open\command" "" '"$INSTDIR\bin\phasordecomp.exe" "%1" %*'

WriteRegStr HKCU "Software\Classes\PhasorVM.PHSB" "" "Phasor Binary File"
WriteRegStr HKCU "Software\Classes\PhasorVM.PHSB\DefaultIcon" "" "$INSTDIR\bin\phasorvm.exe"
WriteRegStr HKCU "Software\Classes\PhasorVM.PHSB\shell\open" "FriendlyAppName" "Phasor VM"
WriteRegStr HKCU "Software\Classes\PhasorVM.PHSB\shell\open\command" "" '"$INSTDIR\bin\phasorvm.exe" "%1" %*'

WriteRegStr HKCU "Software\Classes\.phsb\OpenWithProgids" "Phasor.PHSB"        ""
WriteRegStr HKCU "Software\Classes\.phsb\OpenWithProgids" "PhasorDecomp.PHSB" ""
WriteRegStr HKCU "Software\Classes\.phsb\OpenWithProgids" "PhasorVM.PHSB"     ""


WriteRegStr HKCU "Software\Classes\Phasor.PHS" "" "Phasor Source File"
WriteRegStr HKCU "Software\Classes\Phasor.PHS\DefaultIcon" "" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr HKCU "Software\Classes\Phasor.PHS\shell\open" "FriendlyAppName" "Phasor"
WriteRegStr HKCU "Software\Classes\Phasor.PHS\shell\open\command" "" '"$INSTDIR\${MAIN_APP_EXE}" "%1" %*'

WriteRegStr HKCU "Software\Classes\PhasorCompiler.PHS" "" "Phasor Source File"
WriteRegStr HKCU "Software\Classes\PhasorCompiler.PHS\DefaultIcon" "" "$INSTDIR\bin\phasorcompiler.exe"
WriteRegStr HKCU "Software\Classes\PhasorCompiler.PHS\shell\open" "FriendlyAppName" "Phasor Compiler"
WriteRegStr HKCU "Software\Classes\PhasorCompiler.PHS\shell\open\command" "" '"$INSTDIR\bin\phasorcompiler.exe" "%1" %*'

WriteRegStr HKCU "Software\Classes\PhasorJIT.PHS" "" "Phasor Source File"
WriteRegStr HKCU "Software\Classes\PhasorJIT.PHS\DefaultIcon" "" "$INSTDIR\bin\phasorjit.exe"
WriteRegStr HKCU "Software\Classes\PhasorJIT.PHS\shell\open" "FriendlyAppName" "Phasor JIT"
WriteRegStr HKCU "Software\Classes\PhasorJIT.PHS\shell\open\command" "" '"$INSTDIR\bin\phasorjit.exe" "%1" %*'

WriteRegStr HKCU "Software\Classes\.phs\OpenWithProgids" "Phasor.PHS"          ""
WriteRegStr HKCU "Software\Classes\.phs\OpenWithProgids" "PhasorCompiler.PHS" ""
WriteRegStr HKCU "Software\Classes\.phs\OpenWithProgids" "PhasorJIT.PHS"      ""


WriteRegStr HKCU "Software\Classes\PhasorASM.PHIR" "" "Phasor IR File"
WriteRegStr HKCU "Software\Classes\PhasorASM.PHIR\DefaultIcon" "" "$INSTDIR\bin\phasorasm.exe"
WriteRegStr HKCU "Software\Classes\PhasorASM.PHIR\shell\open" "FriendlyAppName" "Phasor Assembler"
WriteRegStr HKCU "Software\Classes\PhasorASM.PHIR\shell\open\command" "" '"$INSTDIR\bin\phasorasm.exe" "%1" %*'


WriteRegStr HKCU "Software\Classes\.phir\OpenWithProgids" "PhasorASM.PHIR" ""


WriteRegStr HKCU "Software\Classes\Pulsar.PUL" "" "Pulsar Script File"
WriteRegStr HKCU "Software\Classes\Pulsar.PUL\DefaultIcon" "" "$INSTDIR\bin\pulsar.exe"
WriteRegStr HKCU "Software\Classes\Pulsar.PUL\shell\open" "FriendlyAppName" "Pulsar"
WriteRegStr HKCU "Software\Classes\Pulsar.PUL\shell\open\command" "" '"$INSTDIR\bin\pulsar.exe" "%1" %*'

WriteRegStr HKCU "Software\Classes\PulsarCompiler.PUL" "" "Pulsar Script File"
WriteRegStr HKCU "Software\Classes\PulsarCompiler.PUL\DefaultIcon" "" "$INSTDIR\bin\pulsarcompiler.exe"
WriteRegStr HKCU "Software\Classes\PulsarCompiler.PUL\shell\open" "FriendlyAppName" "Pulsar Compiler"
WriteRegStr HKCU "Software\Classes\PulsarCompiler.PUL\shell\open\command" "" '"$INSTDIR\bin\pulsarcompiler.exe" "%1" %*'

WriteRegStr HKCU "Software\Classes\.pul\OpenWithProgids" "Pulsar.PUL"           ""
WriteRegStr HKCU "Software\Classes\.pul\OpenWithProgids" "PulsarCompiler.PUL" ""

EnVar::SetHKLM
EnVar::AddValue "PATH" "$INSTDIR\bin"

System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
SectionEnd

######################################################################

Section Uninstall
${INSTALL_TYPE}
Delete "$INSTDIR\${MAIN_APP_EXE}"
RmDir /r "$INSTDIR"

DeleteRegKey ${REG_ROOT} "${REG_APP_PATH}"
DeleteRegKey ${REG_ROOT} "${UNINSTALL_PATH}"

DeleteRegKey HKCU "Software\Classes\.phir"
DeleteRegKey HKCU "Software\Classes\.phs"
DeleteRegKey HKCU "Software\Classes\.phsb"
DeleteRegKey HKCU "Software\Classes\.pul"

DeleteRegKey HKCU "Software\Classes\Phasor.PHSB"
DeleteRegKey HKCU "Software\Classes\PhasorDecomp.PHSB"
DeleteRegKey HKCU "Software\Classes\PhasorVM.PHSB"
DeleteRegKey HKCU "Software\Classes\Phasor.PHS"
DeleteRegKey HKCU "Software\Classes\PhasorCompiler.PHS"
DeleteRegKey HKCU "Software\Classes\PhasorJIT.PHS"
DeleteRegKey HKCU "Software\Classes\PhasorASM.PHIR"
DeleteRegKey HKCU "Software\Classes\Pulsar.PUL"
DeleteRegKey HKCU "Software\Classes\PulsarCompiler.PUL"

EnVar::SetHKLM
EnVar::DeleteValue "PATH" "$INSTDIR\bin"

System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
SectionEnd

######################################################################
