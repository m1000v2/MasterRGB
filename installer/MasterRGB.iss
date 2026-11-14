#define AppName "MasterRGB"
#define AppVersion "0.2.3-preview"
#define AppPublisher "MasterRGB Project"
#define AppExeName "MasterRGB.exe"

[Setup]
AppId={{6EAA7D5C-7BB8-4A93-B9E2-4E31D0D6B821}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={autopf}\MasterRGB
DefaultGroupName=MasterRGB
OutputDir=..\dist
OutputBaseFilename=MasterRGB-Setup-0.2.3-Preview
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
DisableProgramGroupPage=yes
UninstallDisplayIcon={app}\{#AppExeName}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Tasks]
Name: "desktopicon"; Description: "Criar atalho na área de trabalho"; GroupDescription: "Atalhos:"; Flags: unchecked
Name: "autostart"; Description: "Iniciar MasterRGB com o Windows (com privilégios elevados para acesso à memória)"; GroupDescription: "Inicialização:"; Flags: unchecked

[Files]
Source: "..\dist\MasterRGB-Portable\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\MasterRGB"; Filename: "{app}\Start-MasterRGB.cmd"; WorkingDir: "{app}"
Name: "{autodesktop}\MasterRGB"; Filename: "{app}\Start-MasterRGB.cmd"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{sys}\WindowsPowerShell\v1.0\powershell.exe"; Parameters: "-NoProfile -ExecutionPolicy Bypass -File ""{app}\Set-MasterRGBStartup.ps1"" -Enable -ExePath ""{app}\MasterRGB.exe"" -RunAsAdmin"; Description: "Configurar inicialização automática elevada"; Flags: runhidden; Tasks: autostart
Filename: "powershell.exe"; Parameters: "-NoProfile -ExecutionPolicy Bypass -File ""{app}\Launch-MasterRGB.ps1"""; Description: "Abrir MasterRGB"; Flags: postinstall nowait skipifsilent

[UninstallRun]
Filename: "powershell.exe"; Parameters: "-NoProfile -ExecutionPolicy Bypass -Command ""Unregister-ScheduledTask -TaskName 'MasterRGB Startup' -Confirm:$false -ErrorAction SilentlyContinue"""; Flags: runhidden
