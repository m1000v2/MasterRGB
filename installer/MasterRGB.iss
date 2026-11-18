#define AppName "MasterRGB"
#define AppVersion "0.4.1"
#define AppPublisher "MasterRGB Project"
#define AppExeName "MasterRGB.exe"

[Setup]
AppId={{6EAA7D5C-7BB8-4A93-B9E2-4E31D0D6B821}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppCopyright=Base OpenRGB (GPL-2.0), adaptacao MasterRGB
DefaultDirName={autopf}\MasterRGB
DefaultGroupName=MasterRGB
OutputDir=..\dist
OutputBaseFilename=MasterRGB-Setup-{#AppVersion}
SetupIconFile=..\assets\MasterRGB-icon.ico
UninstallDisplayIcon={app}\{#AppExeName}
UninstallDisplayName={#AppName} {#AppVersion}
VersionInfoVersion=0.4.1.0
VersionInfoProductName={#AppName}
VersionInfoProductVersion={#AppVersion}
VersionInfoCompany={#AppPublisher}
VersionInfoDescription=Instalador do {#AppName}
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
Name: "desktopicon"; Description: "Criar atalho na area de trabalho"; GroupDescription: "Atalhos:"; Flags: unchecked
Name: "autostart"; Description: "Iniciar o MasterRGB junto com o Windows"; GroupDescription: "Inicializacao:"; Flags: unchecked

[Files]
Source: "..\dist\MasterRGB-Portable\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\MasterRGB"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; IconFilename: "{app}\{#AppExeName}"
Name: "{group}\Desinstalar MasterRGB"; Filename: "{uninstallexe}"
Name: "{autodesktop}\MasterRGB"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon; IconFilename: "{app}\{#AppExeName}"
Name: "{userstartup}\MasterRGB"; Filename: "{app}\{#AppExeName}"; Parameters: "--startminimized"; WorkingDir: "{app}"; Tasks: autostart

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Abrir o MasterRGB"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: "{userstartup}\MasterRGB.lnk"
Type: files; Name: "{autodesktop}\MasterRGB.lnk"

[Code]
var
  KeepUserData: Boolean;

function InitializeUninstall(): Boolean;
var
  Answer: Integer;
begin
  Result := True;
  KeepUserData := True;

  if UninstallSilent() then
  begin
    exit;
  end;

  Answer := MsgBox(
    'Deseja manter seus perfis e configuracoes do MasterRGB?' + #13#10 + #13#10 +
    'Sim  = manter os perfis salvos (recomendado)' + #13#10 +
    'Nao  = remover tambem os dados do MasterRGB' + #13#10 +
    'Cancelar = nao desinstalar',
    mbConfirmation, MB_YESNOCANCEL);

  if Answer = IDCANCEL then
  begin
    Result := False;
    exit;
  end;

  KeepUserData := (Answer = IDYES);
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    DeleteFile(ExpandConstant('{userstartup}\MasterRGB.lnk'));

    if not KeepUserData then
    begin
      DelTree(ExpandConstant('{appdata}\MasterRGB'), True, True, True);
      DeleteFile(ExpandConstant('{appdata}\OpenRGB\MasterRGB_LastProfile.txt'));
    end;
  end;
end;
