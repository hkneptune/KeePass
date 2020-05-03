; KeePass Password Safe Installation Script
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

; Thanks to Lubos Stanek for making this installer script multi-language ready

; The name of the application should remain consistent across languages
#define CrossAppName "KeePass"
#define CrossAppFullName "KeePass Password Safe"
#define KeeVersion "1.05"
; VersionInfo cannot contain letters, use 3rd number
#define KeeVVersion "1.0.5.1"

[Languages]
Name: en; MessagesFile: compiler:Default.isl; LicenseFile: ..\Docs\License_Install.txt
Name: cs; MessagesFile: compiler:Languages\Czech.isl; LicenseFile: ..\Docs\License_Install.txt
Name: nl; MessagesFile: compiler:Languages\Dutch.isl; LicenseFile: ..\Docs\License_Install.txt
Name: fr; MessagesFile: compiler:Languages\French.isl; LicenseFile: ..\Docs\License_Install.txt
Name: de; MessagesFile: compiler:Languages\German.isl; LicenseFile: ..\Docs\License_Install.txt

[Setup]
AppName={#CrossAppFullName}
AppVerName={#CrossAppFullName} {#KeeVersion}
AppVersion={#KeeVersion}
AppPublisher=Dominik Reichl
AppPublisherURL=http://keepass.sourceforge.net/
AppSupportURL=http://keepass.sourceforge.net/
AppUpdatesURL=http://keepass.sourceforge.net/
VersionInfoVersion={#KeeVVersion}
DefaultDirName={pf}\{#CrossAppFullName}
DefaultGroupName={#CrossAppFullName}
AllowNoIcons=yes
OutputDir=WinGUI_Distrib
OutputBaseFilename={#CrossAppName}-{#KeeVersion}-Setup
UninstallDisplayIcon={app}\{#CrossAppName}.exe
AppMutex=KeePassApplicationMutex
ChangesAssociations=yes
VersionInfoCompany=Dominik Reichl
VersionInfoDescription={#CrossAppFullName} Setup

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Languages: en cs nl fr de
Name: quicklaunchicon; Description: {cm:CreateQuickLaunchIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked; Languages: en cs nl fr de

[Files]
Source: WinGUI_Distrib\KeePass.exe; DestDir: {app}; Flags: ignoreversion
Source: WinGUI_Distrib\KeePass.chm; DestDir: {app}; Flags: ignoreversion
Source: WinGUI_Distrib\KeePass.ini; DestDir: {app}; Flags: ignoreversion onlyifdoesntexist
Source: WinGUI_Distrib\License.txt; DestDir: {app}; Flags: ignoreversion

[INI]
Filename: {app}\{#CrossAppName}.url; Section: InternetShortcut; Key: URL; String: http://keepass.sourceforge.net/
; Filename: "{app}\KeePass.ini"; Section: "KeePass"; Key: "KeeLanguage"; String: "Standard"; Languages: en

[Icons]
Name: {group}\{#CrossAppFullName}; Filename: {app}\{#CrossAppName}.exe; Languages: en cs nl fr de
Name: {group}\{cm:ProgramOnTheWeb,{#CrossAppFullName}}; Filename: {app}\{#CrossAppName}.url; Languages: en cs nl fr de
Name: {group}\{cm:UninstallProgram,{#CrossAppFullName}}; Filename: {uninstallexe}; Languages: en cs nl fr de
Name: {userdesktop}\{#CrossAppFullName}; Filename: {app}\{#CrossAppName}.exe; Tasks: desktopicon; Languages: en cs nl fr de
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\{#CrossAppFullName}; Filename: {app}\{#CrossAppName}.exe; Tasks: quicklaunchicon; Languages: en cs nl fr de

[Run]
Filename: {app}\{#CrossAppName}.exe; Description: {cm:LaunchProgram,{#CrossAppFullName}}; Flags: nowait postinstall skipifsilent; Languages: en cs nl fr de

; A hack to unregister .kdb association
[Registry]
Root: HKCR; Subkey: .kdb; Flags: uninsdeletekey
Root: HKCR; Subkey: kdbfile; Flags: uninsdeletekey

[UninstallDelete]
Type: files; Name: {app}\{#CrossAppName}.url