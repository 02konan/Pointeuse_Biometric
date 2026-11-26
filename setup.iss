#define AppName "Divix Biometric"
#define AppVersion "1.0.0"
#define AppPublisher "IFSM"
#define AppExeName "Divix.exe"
#define BuildDir "dist\Divix"

[Setup]
AppId={{66E9C0E1-5D78-4D47-91E8-3126CFF2E07F}}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={localappdata}\Programs\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
SetupIconFile=static\images\icons8-fingerprint-50.ico
UninstallDisplayIcon={app}\{#AppExeName}
OutputDir=dist\installer
OutputBaseFilename=Setup-Divix-Biometric
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

[Dirs]
Name: "{app}\uploads"

[Files]
Source: "{#BuildDir}\{#AppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\.env"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\_internal\*"; DestDir: "{app}\_internal"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Créer un raccourci sur le bureau"; GroupDescription: "Raccourcis supplémentaires :"; Flags: unchecked

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Lancer {#AppName}"; Flags: postinstall nowait skipifsilent
