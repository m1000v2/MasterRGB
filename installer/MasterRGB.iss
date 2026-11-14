#define AppName "MasterRGB"
#define AppVersion "0.3.0"
#define AppPublisher "MasterRGB Project"
#define AppExeName "MasterRGB.exe"

[Setup]
AppId={{6EAA7D5C-7BB8-4A93-B9E2-4E31D0D6B821}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={autopf}\MasterRGB
DefaultGroupName=MasterRGB
OutputDir=..\dist
OutputBaseFilename=MasterRGB-Setup-0.3.0
SetupIconFile=..\assets\MasterRGB-icon.ico
UninstallDisplayIcon={app}\{#AppExeName}
ArchitecturesInstallIn64BitMode=x64compatible
ArchitecturesAllowed=x64compatible
PrivilegesRequired=admin
DisableProgramGroupPage=yes
WizardStyle=modern
Compression=lzma2
SolidCompression=yes
CloseApplications=yes
RestartApplications=no
MinVersion=10.0

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Tasks]
Name: "desktopicon"; Description: "Criar atalho na área de trabalho"; GroupDescription: "Atalhos:"; Flags: unchecked
Name: "autostart"; Description: "Iniciar o MasterRGB junto com o Windows"; GroupDescription: "Inicialização:"; Flags: unchecked

[Files]
Source: "..\dist\MasterRGB-Portable\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\MasterRGB"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"
Name: "{group}\Desinstalar MasterRGB"; Filename: "{uninstallexe}"
Name: "{autodesktop}\MasterRGB"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon
Name: "{userstartup}\MasterRGB"; Filename: "{app}\{#AppExeName}"; Parameters: "--startminimized"; WorkingDir: "{app}"; Tasks: autostart

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Abrir o MasterRGB"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{userstartup}\MasterRGB.lnk"
