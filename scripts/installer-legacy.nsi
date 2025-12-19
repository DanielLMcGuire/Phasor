!include "x64.nsh"
!define APP_NAME "Phasor Programming Language"
!define COMP_NAME "Daniel McGuire"
!define VERSION "1.0.0.0"
!define COPYRIGHT "(C) 2025 Daniel McGuire"
!define LICENSE_TXT "license.txt"
!define DESCRIPTION "Functional VM Compiled Programming Language"
!define INSTALLER_NAME "phasor-x64rel-legacy-setup.exe"
!define MAIN_APP_EXE "phasorvm.exe"
!define INSTALL_TYPE "SetShellVarContext all"
!define REG_ROOT "HKLM"
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
InstallDirRegKey "${REG_ROOT}" "${REG_APP_PATH}" ""
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
file /r "..\install\legacy\bin"


SectionEnd

######################################################################

Section -Icons_Reg
SetOutPath "$INSTDIR"

WriteUninstaller "$INSTDIR\uninstall.exe"

WriteRegStr ${REG_ROOT} "${REG_APP_PATH}" "" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayName" "${APP_NAME}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "UninstallString" "$INSTDIR\uninstall.exe"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayIcon" "$INSTDIR\${MAIN_APP_EXE}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "DisplayVersion" "${VERSION}"
WriteRegStr ${REG_ROOT} "${UNINSTALL_PATH}"  "Publisher" "${COMP_NAME}"

; Ensure 64-bit registry view
SetRegView 64
ClearErrors

; Read Installed DWORD
ReadRegDWORD $R0 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Installed"
SetRegView 32 ; restore default

; Check if read failed or value is not 1
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


System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
SectionEnd

######################################################################

Section Uninstall
${INSTALL_TYPE}
Delete "$INSTDIR\${MAIN_APP_EXE}"
RmDir /r "$INSTDIR"

DeleteRegKey ${REG_ROOT} "${REG_APP_PATH}"
DeleteRegKey ${REG_ROOT} "${UNINSTALL_PATH}"

SectionEnd

######################################################################
