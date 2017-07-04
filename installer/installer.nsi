;--------------------------------
;Include Modern UI

	!include "MUI2.nsh"

;--------------------------------
;General

	!define RECORDER_BASEDIR "..\bin\Release"
	!define DRIVER_BASEDIR "..\thirdparty\OpenVR-InputEmulator\driver_vrinputemulator"

	;Name and file
	Name "OpenVR Input Recorder"
	OutFile "openvr-input-recorder-installer.exe"

	;Default installation folder
	InstallDir "$PROGRAMFILES64\OpenVR-Input-Recorder"

	;Get installation folder from registry if available
	InstallDirRegKey HKLM "Software\OpenVR-Input-Recorder\CommandLine" ""

	;Request application privileges for Windows Vista
	RequestExecutionLevel admin

;--------------------------------
;Variables

VAR upgradeInstallation

;--------------------------------
;Interface Settings

	!define MUI_ABORTWARNING

;--------------------------------
;Pages

	!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
	!define MUI_PAGE_CUSTOMFUNCTION_PRE dirPre
	!insertmacro MUI_PAGE_DIRECTORY
	!insertmacro MUI_PAGE_INSTFILES

	!insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

	!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Macros

;--------------------------------
;Functions

Function dirPre
	StrCmp $upgradeInstallation "true" 0 +2
		Abort
FunctionEnd

Function .onInit
	StrCpy $upgradeInstallation "false"

	ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenVR-Input-Recorder" "UninstallString"
	StrCmp $R0 "" done


	; If SteamVR is already running, display a warning message and exit
	FindWindow $0 "Qt5QWindowIcon" "SteamVR Status"
	StrCmp $0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION \
			"SteamVR is still running. Cannot install this software.$\nPlease close SteamVR and try again."
		Abort


	MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
		"OpenVR Input Recorder is already installed. $\n$\nClick `OK` to upgrade the \
		existing installation or `Cancel` to cancel this upgrade." \
		IDOK upgrade
	Abort

	upgrade:
		StrCpy $upgradeInstallation "true"
	done:
FunctionEnd

;--------------------------------
;Installer Sections

Section "Install" SecInstall

	StrCmp $upgradeInstallation "true" 0 noupgrade
		DetailPrint "Uninstall previous version..."
		ExecWait '"$INSTDIR\Uninstall.exe" /S _?=$INSTDIR'
		Delete $INSTDIR\Uninstall.exe
		Goto afterupgrade

	noupgrade:

	afterupgrade:

	SetOutPath "$INSTDIR"

	;ADD YOUR OWN FILES HERE...
	File "${RECORDER_BASEDIR}\*.exe"
	File "${RECORDER_BASEDIR}\*.dll"

	; Install redistributable
	; ExecWait '"$INSTDIR\vcredist_x64.exe" /install /quiet'

	Var /GLOBAL vrRuntimePath
	; nsExec::ExecToStack '"$INSTDIR\OpenVR-InputEmulatorOverlay.exe" -openvrpath'
	; Pop $0
	; Pop $vrRuntimePath
	StrCpy $vrRuntimePath "C:\Program Files (x86)\Steam\steamapps\common\SteamVR"
	DetailPrint "VR runtime path: $vrRuntimePath"

	SetOutPath "$vrRuntimePath\drivers\00vrinputemulator"
	File "${DRIVER_BASEDIR}\driver.vrdrivermanifest"
	SetOutPath "$vrRuntimePath\drivers\00vrinputemulator\resources"
	File "${DRIVER_BASEDIR}\resources\driver.vrresources"
	SetOutPath "$vrRuntimePath\drivers\00vrinputemulator\resources\settings"
	File "${DRIVER_BASEDIR}\resources\settings\default.vrsettings"
	SetOutPath "$vrRuntimePath\drivers\00vrinputemulator\bin\win64"
	File "${DRIVER_BASEDIR}\bin\x64\driver_00vrinputemulator.dll"

	;Store installation folder
	WriteRegStr HKLM "Software\OpenVR-Input-Recorder\CommandLine" "" $INSTDIR
	WriteRegStr HKLM "Software\OpenVR-Input-Recorder\Driver" "" $vrRuntimePath

	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenVR-Input-Recorder" "DisplayName" "OpenVR Input Recorder"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenVR-Input-Recorder" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"
	; If SteamVR is already running, display a warning message and exit
	FindWindow $0 "Qt5QWindowIcon" "SteamVR Status"
	StrCmp $0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION \
			"SteamVR is still running. Cannot uninstall this software.$\nPlease close SteamVR and try again."
		Abort

	; Delete installed files
	Var /GLOBAL vrRuntimePath2
	ReadRegStr $vrRuntimePath2 HKLM "Software\OpenVR-Input-Recorder\Driver" ""
	DetailPrint "VR runtime path: $vrRuntimePath2"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\driver.vrdrivermanifest"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\resources\driver.vrresources"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\resources\settings\default.vrsettings"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\bin\win64\driver_00vrinputemulator.dll"
	Delete "$vrRuntimePath2\drivers\00vrinputemulator\bin\win64\driver_vrinputemulator.log"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\resources\settings"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\resources\"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\bin\win64\"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\bin\"
	RMdir "$vrRuntimePath2\drivers\00vrinputemulator\"

	Delete "$INSTDIR\openvr_api.dll"
	Delete "$INSTDIR\openvr-input-recorder.exe"
	Delete "$INSTDIR\Uninstall.exe"
	RMdir "$INSTDIR\"

	DeleteRegKey HKLM "Software\OpenVR-Input-Recorder\CommandLine"
	DeleteRegKey HKLM "Software\OpenVR-Input-Recorder\Driver"
	DeleteRegKey HKLM "Software\OpenVR-Input-Recorder"
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenVR-Input-Recorder"
SectionEnd
