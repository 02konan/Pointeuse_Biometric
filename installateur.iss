; ============================================
; DIVIX-BIOMETRIC - INSTALLATEUR WINDOWS
; ============================================
; À compiler avec Inno Setup 6, depuis la racine du projet, APRÈS
;     pyinstaller --noconfirm app.spec
; qui produit le dossier dist\Divix\ (Divix.exe + _internal\ + .env).

#define MyAppName "Divix-Biometric"
#define MyAppVersion "2.0.0"
#define MyAppPublisher "Divix"
#define MyAppExeName "Divix.exe"
; Dossier produit par PyInstaller, relatif à ce fichier .iss.
#define BuildDir AddBackslash(SourcePath) + "dist\Divix"

#if !FileExists(BuildDir + "\" + MyAppExeName)
  #error "dist\Divix\Divix.exe introuvable : lancez d'abord « pyinstaller --noconfirm app.spec »."
#endif

[Setup]
AppId={{03D2EF39-D300-469B-B2B8-ECB9AEC27DD4}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}

DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes

UninstallDisplayIcon={app}\{#MyAppExeName}
UninstallDisplayName={#MyAppName}

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

WizardStyle=modern
PrivilegesRequired=admin

; Ferme Divix s'il tourne pendant une mise à jour.
CloseApplications=yes

OutputDir=Output
OutputBaseFilename=Divix-Biometric-Setup-{#MyAppVersion}

Compression=lzma2
SolidCompression=yes

; ============================================
; LANGUE
; ============================================

[Languages]
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

; ============================================
; OPTIONS D'INSTALLATION
; ============================================

[Tasks]
Name: "desktopicon"; Description: "Créer un raccourci sur le Bureau"; GroupDescription: "Options supplémentaires :"; Flags: unchecked

; ============================================
; FICHIERS DE L'APPLICATION
; ============================================

[Files]
; Contenu de dist\Divix (et non de dist\) : Divix.exe doit arriver
; directement dans {app}, là où pointent les raccourcis.
Source: "{#BuildDir}\*"; DestDir: "{app}"; Excludes: ".env"; Flags: recursesubdirs createallsubdirs ignoreversion
; Configuration : installée une seule fois, jamais écrasée par une mise à
; jour ni supprimée à la désinstallation (elle contient les réglages du site).
Source: "{#BuildDir}\.env"; DestDir: "{app}"; Flags: onlyifdoesntexist uninsneveruninstall skipifsourcedoesntexist

; ============================================
; DOSSIERS ACCESSIBLES EN ÉCRITURE
; ============================================
; Sous Program Files, un utilisateur standard ne peut pas écrire : sans ces
; droits, la génération des rapports PDF échouerait.

[Dirs]
Name: "{app}\_internal\uploads"; Permissions: users-modify
Name: "{app}\_internal\programme\uploads"; Permissions: users-modify

; ============================================
; RACCOURCIS
; ============================================

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

; ============================================
; LANCEMENT APRES INSTALLATION
; ============================================

[Run]
; runasoriginaluser : sans lui, l'application serait lancée avec les droits
; administrateur de l'installateur.
Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Description: "Lancer {#MyAppName}"; Flags: nowait postinstall skipifsilent runasoriginaluser

; ============================================
; DÉSINSTALLATION
; ============================================
; Fichiers créés après l'installation (rapports PDF, caches Python) : sans
; cela, le dossier {app} resterait après la désinstallation.

[UninstallDelete]
Type: filesandordirs; Name: "{app}\_internal\uploads"
Type: filesandordirs; Name: "{app}\_internal\programme\uploads"
Type: filesandordirs; Name: "{app}\_internal\__pycache__"
