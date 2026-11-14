[CmdletBinding()]
param(
    [switch]$StartupLaunched,
    [switch]$Quiet
)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
[System.Windows.Forms.Application]::EnableVisualStyles()

$AppTitle = 'MasterRGB'
$AppVersion = '0.2.3-preview'
$AppDir = $PSScriptRoot
$Exe = Join-Path $AppDir 'MasterRGB.exe'
$ConfigDir = Join-Path $env:APPDATA 'MasterRGB'
$SettingsFile = Join-Path $ConfigDir 'settings.json'
$ThemeFile = Join-Path $ConfigDir 'theme.txt'
$SupportFile = Join-Path $ConfigDir 'support.json'
$OpenRgbProfileDir = Join-Path $env:APPDATA 'OpenRGB'
$LastProfileFile = Join-Path $OpenRgbProfileDir 'MasterRGB_LastProfile.txt'
$TaskName = 'MasterRGB Startup'
$StartupLog = Join-Path $ConfigDir 'startup.log'

New-Item -ItemType Directory -Force -Path $ConfigDir | Out-Null
$defaultSupport = Join-Path $AppDir 'config\support.json'
if (-not (Test-Path $SupportFile) -and (Test-Path $defaultSupport)) {
    Copy-Item $defaultSupport $SupportFile -Force
}

$Defaults = [ordered]@{
    theme = 'Dark'
    runAsAdmin = $true
    startMinimized = $true
    startWithWindows = $false
    profileName = ''
    profileDisplayName = ''
    lastVersion = $AppVersion
}
$Settings = [ordered]@{}
foreach ($key in $Defaults.Keys) { $Settings[$key] = $Defaults[$key] }
if (Test-Path $SettingsFile) {
    try {
        $saved = Get-Content -Raw -Path $SettingsFile | ConvertFrom-Json
        foreach ($key in $Defaults.Keys) {
            $prop = $saved.PSObject.Properties[$key]
            if ($null -ne $prop) { $Settings[$key] = $prop.Value }
        }
        # Migrate old 0.1 path setting to the profile basename when possible.
        if (-not $Settings.profileName -and $saved.PSObject.Properties['profilePath'] -and $saved.profilePath) {
            $legacyPath = [string]$saved.profilePath
            if (Test-Path -LiteralPath $legacyPath) {
                $legacyName = [IO.Path]::GetFileNameWithoutExtension($legacyPath)
                $legacyExt = [IO.Path]::GetExtension($legacyPath).ToLowerInvariant()
                if ($legacyExt -eq '.orp') {
                    New-Item -ItemType Directory -Force -Path $OpenRgbProfileDir | Out-Null
                    $target = Join-Path $OpenRgbProfileDir ($legacyName + '.orp')
                    if (-not (Test-Path $target)) { Copy-Item $legacyPath $target }
                    $Settings.profileName = $legacyName
                    $Settings.profileDisplayName = $legacyName
                }
            }
        }
    } catch {
        Write-Warning "Configuração inválida; serão usados os padrões. $($_.Exception.Message)"
    }
}
if ($Settings.theme -notin @('Dark','Light')) { $Settings.theme = 'Dark' }
Set-Content -Path $ThemeFile -Value $Settings.theme -Encoding ASCII

function Save-Settings {
    param([System.Collections.IDictionary]$Value)
    $Value.lastVersion = $AppVersion
    $Value | ConvertTo-Json -Depth 5 | Set-Content -Path $SettingsFile -Encoding UTF8
    Set-Content -Path $ThemeFile -Value $Value.theme -Encoding ASCII
}
function Write-StartupLog([string]$Message) {
    $line = ('[{0}] {1}' -f (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'), $Message)
    Add-Content -Path $StartupLog -Value $line -Encoding UTF8
}
function Get-LastUsedProfile {
    if (-not (Test-Path -LiteralPath $LastProfileFile)) { return '' }
    try {
        $name = (Get-Content -LiteralPath $LastProfileFile -Raw -ErrorAction Stop).Trim()
        if (-not $name) { return '' }
        if ($name.EndsWith('.orp', [StringComparison]::OrdinalIgnoreCase)) {
            $name = $name.Substring(0, $name.Length - 4)
        }
        return $name
    } catch {
        Write-StartupLog ('Não foi possível ler o último perfil salvo: ' + $_.Exception.Message)
        return ''
    }
}
function Get-ProfileArguments {
    param([string]$ProfileName, [bool]$Minimized)
    $parts = @('--gui')
    if ($ProfileName) {
        # A branch used by this preview stores .orp files in %APPDATA%\OpenRGB.
        # The CLI expects the profile basename, without the extension.
        $parts += '--profile'
        $parts += ('"' + $ProfileName.Replace('"','') + '"')
    }
    if ($Minimized) { $parts += '--startminimized' }
    return ($parts -join ' ')
}
function Start-MasterRGB {
    if (-not (Test-Path -LiteralPath $Exe)) {
        [System.Windows.Forms.MessageBox]::Show("MasterRGB.exe não foi encontrado em:`n$AppDir", $AppTitle, 'OK', 'Error') | Out-Null
        return
    }
    # Manual launch also defaults to the last profile actually applied in the RGB core.
    $profileToLoad = Get-LastUsedProfile
    if (-not $profileToLoad) { $profileToLoad = [string]$Settings.profileName }
    if ($profileToLoad -and -not (Test-Path (Join-Path $OpenRgbProfileDir ($profileToLoad + '.orp')))) {
        [System.Windows.Forms.MessageBox]::Show("O último perfil registrado não foi encontrado em:`n$OpenRgbProfileDir", 'Perfil não encontrado', 'OK', 'Warning') | Out-Null
        return
    }
    # Manual launch always opens the control window; startMinimized applies only to logon startup.
    $arguments = Get-ProfileArguments -ProfileName $profileToLoad -Minimized $false
    try {
        if ([bool]$Settings.runAsAdmin) {
            Start-Process -FilePath $Exe -ArgumentList $arguments -WorkingDirectory $AppDir -Verb RunAs
        } else {
            Start-Process -FilePath $Exe -ArgumentList $arguments -WorkingDirectory $AppDir
        }
        if (-not $Quiet) { $form.Close() }
    } catch {
        [System.Windows.Forms.MessageBox]::Show($_.Exception.Message, 'Falha ao iniciar MasterRGB', 'OK', 'Error') | Out-Null
    }
}
function Invoke-StartupConfiguration {
    param([bool]$Enable)
    $script = Join-Path $AppDir 'Set-MasterRGBStartup.ps1'
    if (-not (Test-Path $script)) {
        [System.Windows.Forms.MessageBox]::Show('O módulo de inicialização não foi encontrado.', $AppTitle, 'OK', 'Error') | Out-Null
        return $false
    }
    $verb = if ($Enable) { '-Enable' } else { '-Disable' }
    $adminArg = if ([bool]$Settings.runAsAdmin) { ' -RunAsAdmin' } else { '' }
    $args = '-NoProfile -ExecutionPolicy Bypass -File "' + $script + '" ' + $verb
    if ($Enable) { $args += ' -ExePath "' + $Exe + '"' + $adminArg }
    try {
        $p = Start-Process -FilePath 'powershell.exe' -Verb RunAs -ArgumentList $args -Wait -PassThru
        return ($p.ExitCode -eq 0)
    } catch {
        [System.Windows.Forms.MessageBox]::Show($_.Exception.Message, 'Permissão necessária', 'OK', 'Warning') | Out-Null
        return $false
    }
}
function Get-StartupEnabled {
    try { return ($null -ne (Get-ScheduledTask -TaskName $TaskName -ErrorAction Stop)) }
    catch { return $false }
}
function Get-SupportConfig {
    if (Test-Path $SupportFile) {
        try { return (Get-Content -Raw $SupportFile | ConvertFrom-Json) } catch {}
    }
    return [pscustomobject]@{ paypalUrl=''; githubRepository=''; githubUrl='' }
}
function Save-SupportConfig([string]$PaypalUrl, [string]$Repository, [string]$GithubUrl) {
    $obj = [ordered]@{
        product = 'MasterRGB'
        paypalUrl = $PaypalUrl.Trim()
        paypalQrFile = 'paypal-qr.png'
        githubRepository = $Repository.Trim()
        githubUrl = $GithubUrl.Trim()
        supportLabel = 'Apoiar o MasterRGB'
    }
    $obj | ConvertTo-Json | Set-Content -Path $SupportFile -Encoding UTF8
}
function New-DiagnosticReport {
    $desktop = [Environment]::GetFolderPath('Desktop')
    $path = Join-Path $desktop ('MasterRGB-Diagnostico-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.txt')
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.Add('MASTER RGB — RELATÓRIO DE DIAGNÓSTICO')
    $lines.Add(('Data: ' + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss zzz')))
    $lines.Add(('Versão do MasterRGB: ' + $AppVersion))
    $lines.Add(('Windows: ' + [Environment]::OSVersion.VersionString))
    $arch = if ([Environment]::Is64BitOperatingSystem) { '64-bit' } else { '32-bit' }
    $lines.Add(('Arquitetura: ' + $arch))
    $installMode = if ($AppDir.StartsWith($env:ProgramFiles,[StringComparison]::OrdinalIgnoreCase)) { 'Instalável' } else { 'Portable' }
    $lines.Add(('Modo de instalação: ' + $installMode))
    $lines.Add(('Inicialização elevada configurada: ' + [bool]$Settings.runAsAdmin))
    $lines.Add(('Tarefa de inicialização presente: ' + (Get-StartupEnabled)))
    $lines.Add(('Perfil configurado: ' + [string]$Settings.profileName))
    $lines.Add('')
    foreach ($query in @(
        @{ title='PLACA-MÃE'; class='Win32_BaseBoard'; props=@('Manufacturer','Product','Version') },
        @{ title='GPU'; class='Win32_VideoController'; props=@('Name','PNPDeviceID','DriverVersion') },
        @{ title='MEMÓRIA'; class='Win32_PhysicalMemory'; props=@('Manufacturer','PartNumber','Capacity','Speed') }
    )) {
        $lines.Add('--- ' + $query.title + ' ---')
        try {
            $items = Get-CimInstance -ClassName $query.class -ErrorAction Stop
            foreach ($item in $items) {
                foreach ($prop in $query.props) { $lines.Add(($prop + ': ' + [string]$item.$prop)) }
                $lines.Add('')
            }
        } catch { $lines.Add('Falha ao consultar: ' + $_.Exception.Message) }
    }
    $lines.Add('--- DRIVER PAWNIO ---')
    try {
        $driver = Get-CimInstance Win32_SystemDriver -Filter "Name='PawnIO'" -ErrorAction Stop
        if ($driver) { $lines.Add(('Estado: ' + $driver.State)); $lines.Add(('Inicialização: ' + $driver.StartMode)); $lines.Add(('Caminho: ' + $driver.PathName)) }
        else { $lines.Add('PawnIO não encontrado nesta consulta.') }
    } catch { $lines.Add('Falha ao consultar PawnIO: ' + $_.Exception.Message) }
    $lines.Add('')
    $lines.Add('Privacidade: este relatório é salvo localmente. Nada é enviado automaticamente.')
    $lines | Set-Content -Path $path -Encoding UTF8
    return $path
}
function Open-Support {
    $support = Get-SupportConfig
    $supportForm = New-Object System.Windows.Forms.Form
    $supportForm.Text = 'MasterRGB — Suporte e Apoio'
    $supportForm.StartPosition = 'CenterParent'
    $supportForm.Size = New-Object System.Drawing.Size(690, 500)
    $supportForm.FormBorderStyle = 'FixedDialog'
    $supportForm.MaximizeBox = $false
    $supportForm.BackColor = $colors.bg
    $supportForm.ForeColor = $colors.text
    $supportForm.Font = New-Object System.Drawing.Font('Segoe UI', 9)
if (Test-Path $iconPath) { try { $supportForm.Icon = New-Object System.Drawing.Icon($iconPath) } catch {} }
    $lbl = New-Object System.Windows.Forms.Label
    $lbl.Text = 'SUPORTE DO MASTERRGB'; $lbl.Location = New-Object System.Drawing.Point(24,20); $lbl.Size = New-Object System.Drawing.Size(600,30)
    $lbl.Font = New-Object System.Drawing.Font('Segoe UI Semibold',16,[System.Drawing.FontStyle]::Bold); $lbl.ForeColor=$colors.accent2; $supportForm.Controls.Add($lbl)
    $desc = New-Object System.Windows.Forms.Label
    $desc.Text = 'Ajude a melhorar o projeto, consulte atualizações ou gere um diagnóstico local para anexar a um pedido de suporte.'
    $desc.Location = New-Object System.Drawing.Point(26,58); $desc.Size = New-Object System.Drawing.Size(620,42); $desc.ForeColor=$colors.muted; $supportForm.Controls.Add($desc)
    $paypalLabel=New-Object System.Windows.Forms.Label; $paypalLabel.Text='Link PayPal / PayPal.Me'; $paypalLabel.Location=New-Object System.Drawing.Point(26,117); $paypalLabel.Size=New-Object System.Drawing.Size(220,22); $paypalLabel.ForeColor=$colors.text; $supportForm.Controls.Add($paypalLabel)
    $paypal=New-Object System.Windows.Forms.TextBox; $paypal.Location=New-Object System.Drawing.Point(26,142); $paypal.Size=New-Object System.Drawing.Size(620,26); $paypal.Text=[string]$support.paypalUrl; $supportForm.Controls.Add($paypal)
    $repoLabel=New-Object System.Windows.Forms.Label; $repoLabel.Text='Repositório GitHub (proprietário/MasterRGB)'; $repoLabel.Location=New-Object System.Drawing.Point(26,184); $repoLabel.Size=New-Object System.Drawing.Size(300,22); $repoLabel.ForeColor=$colors.text; $supportForm.Controls.Add($repoLabel)
    $repo=New-Object System.Windows.Forms.TextBox; $repo.Location=New-Object System.Drawing.Point(26,209); $repo.Size=New-Object System.Drawing.Size(300,26); $repo.Text=[string]$support.githubRepository; $supportForm.Controls.Add($repo)
    $github=New-Object System.Windows.Forms.TextBox; $github.Location=New-Object System.Drawing.Point(340,209); $github.Size=New-Object System.Drawing.Size(306,26); $github.Text=[string]$support.githubUrl; $supportForm.Controls.Add($github)
    $saveSupport=New-Object System.Windows.Forms.Button; $saveSupport.Text='SALVAR DADOS'; $saveSupport.Location=New-Object System.Drawing.Point(26,250); $saveSupport.Size=New-Object System.Drawing.Size(140,38); $supportForm.Controls.Add($saveSupport)
    $donate=New-Object System.Windows.Forms.Button; $donate.Text='APOIAR VIA PAYPAL'; $donate.Location=New-Object System.Drawing.Point(176,250); $donate.Size=New-Object System.Drawing.Size(170,38); $supportForm.Controls.Add($donate)
    $openGithub=New-Object System.Windows.Forms.Button; $openGithub.Text='ABRIR GITHUB'; $openGithub.Location=New-Object System.Drawing.Point(356,250); $openGithub.Size=New-Object System.Drawing.Size(130,38); $supportForm.Controls.Add($openGithub)
    $diag=New-Object System.Windows.Forms.Button; $diag.Text='GERAR DIAGNÓSTICO'; $diag.Location=New-Object System.Drawing.Point(26,305); $diag.Size=New-Object System.Drawing.Size(180,38); $supportForm.Controls.Add($diag)
    $openReadme=New-Object System.Windows.Forms.Button; $openReadme.Text='ABRIR MANUAL'; $openReadme.Location=New-Object System.Drawing.Point(216,305); $openReadme.Size=New-Object System.Drawing.Size(150,38); $supportForm.Controls.Add($openReadme)
    $close=New-Object System.Windows.Forms.Button; $close.Text='FECHAR'; $close.Location=New-Object System.Drawing.Point(516,405); $close.Size=New-Object System.Drawing.Size(130,36); $supportForm.Controls.Add($close)
    $note=New-Object System.Windows.Forms.Label; $note.Text='O diagnóstico é salvo na Área de Trabalho. Revise o arquivo antes de compartilhar. Nenhum dado é enviado automaticamente.'; $note.Location=New-Object System.Drawing.Point(26,365); $note.Size=New-Object System.Drawing.Size(620,38); $note.ForeColor=$colors.muted; $supportForm.Controls.Add($note)
    foreach($ctrl in @($paypal,$repo,$github)){ $ctrl.BackColor=$colors.panel2; $ctrl.ForeColor=$colors.text }
    foreach($btn in @($saveSupport,$donate,$openGithub,$diag,$openReadme,$close)){ $btn.FlatStyle='Flat'; $btn.BackColor=$colors.panel2; $btn.ForeColor=$colors.text; $btn.FlatAppearance.BorderColor=$colors.line }
    $saveSupport.Add_Click({
        if($paypal.Text -and $paypal.Text -notmatch '^https://(www\.)?paypal\.com/|^https://paypal\.me/') { [System.Windows.Forms.MessageBox]::Show('Informe um link HTTPS válido do PayPal ou deixe o campo vazio.','Link inválido','OK','Warning')|Out-Null; return }
        if($repo.Text -and $repo.Text -notmatch '^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$') { [System.Windows.Forms.MessageBox]::Show('Use o formato proprietário/MasterRGB para o repositório.','GitHub inválido','OK','Warning')|Out-Null; return }
        Save-SupportConfig -PaypalUrl $paypal.Text -Repository $repo.Text -GithubUrl $github.Text
        [System.Windows.Forms.MessageBox]::Show('Dados de suporte salvos.','MasterRGB','OK','Information')|Out-Null
    })
    $donate.Add_Click({
        if($paypal.Text -match '^https://(www\.)?paypal\.com/|^https://paypal\.me/') { Start-Process $paypal.Text }
        else {
            $qrPath = Join-Path $ConfigDir 'paypal-qr.png'
            if (-not (Test-Path $qrPath)) { $qrPath = Join-Path $AppDir 'config\paypal-qr.png' }
            if (Test-Path $qrPath) { Start-Process $qrPath }
            else { [System.Windows.Forms.MessageBox]::Show('O campo PayPal ainda precisa receber o mesmo link usado no YT Desk Monitor ou uma imagem QR em config\paypal-qr.png. Não foi configurado um endereço fictício.','Apoiar o MasterRGB','OK','Information')|Out-Null }
        }
    })
    $openGithub.Add_Click({
        if($github.Text -match '^https://github\.com/') { Start-Process $github.Text }
        elseif($repo.Text -match '^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$'){ Start-Process ('https://github.com/' + $repo.Text) }
        else { [System.Windows.Forms.MessageBox]::Show('O repositório oficial ainda não foi definido.','GitHub','OK','Information')|Out-Null }
    })
    $diag.Add_Click({
        try { $report=New-DiagnosticReport; [System.Windows.Forms.MessageBox]::Show("Relatório criado:`n$report`n`nRevise antes de compartilhar.",'Diagnóstico','OK','Information')|Out-Null; Start-Process notepad.exe -ArgumentList ('"' + $report + '"') }
        catch { [System.Windows.Forms.MessageBox]::Show($_.Exception.Message,'Falha no diagnóstico','OK','Error')|Out-Null }
    })
    $openReadme.Add_Click({ $readme=Join-Path $AppDir 'README-PTBR.md'; if(Test-Path $readme){Start-Process $readme}else{[System.Windows.Forms.MessageBox]::Show('O manual não foi encontrado nesta pasta.','Ajuda','OK','Information')|Out-Null} })
    $close.Add_Click({ $supportForm.Close() })
    [void]$supportForm.ShowDialog($form)
}
function Check-Updates {
    $support = Get-SupportConfig
    $repo = [string]$support.githubRepository
    if ($repo -notmatch '^[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+$') {
        [System.Windows.Forms.MessageBox]::Show('O repositório ainda não foi configurado. Abra Suporte e informe proprietário/MasterRGB quando o GitHub estiver publicado.','Atualizações','OK','Information')|Out-Null
        return
    }
    try {
        $releases = @(Invoke-RestMethod -Uri "https://api.github.com/repos/$repo/releases?per_page=10" -Headers @{ 'User-Agent'='MasterRGB-Updater'; 'Accept'='application/vnd.github+json' } -TimeoutSec 15)
        $latest = $releases | Where-Object { -not $_.draft } | Select-Object -First 1
        if (-not $latest) { [System.Windows.Forms.MessageBox]::Show('O repositório ainda não publicou Releases.','Atualizações','OK','Information')|Out-Null; return }
        $tag = [string]$latest.tag_name
        $normalizedTag = $tag -replace '^v',''
        $normalizedCurrent = $AppVersion -replace '^v',''
        if ($normalizedTag -eq $normalizedCurrent) {
            [System.Windows.Forms.MessageBox]::Show("Você já está usando $AppVersion.",'MasterRGB atualizado','OK','Information')|Out-Null
            return
        }
        $answer=[System.Windows.Forms.MessageBox]::Show("Versão publicada: $tag`nVersão instalada: $AppVersion`n`nAbrir a página da Release para conferir os arquivos e as notas?",'Atualização disponível','YesNo','Information')
        if($answer -eq [System.Windows.Forms.DialogResult]::Yes -and $latest.html_url){Start-Process ([string]$latest.html_url)}
    } catch {
        [System.Windows.Forms.MessageBox]::Show("Não foi possível consultar o GitHub agora.`n$($_.Exception.Message)",'Atualizações','OK','Warning')|Out-Null
    }
}

# A scheduled task runs this launcher elevated/limited at logon. The core is
# started as a child process and inherits the task's token.
if ($StartupLaunched) {
    if (-not (Test-Path -LiteralPath $Exe)) { Write-StartupLog 'ERRO: MasterRGB.exe não encontrado.'; exit 2 }

    # The core writes MasterRGB_LastProfile.txt only after a profile loads
    # successfully, including profile changes made later inside the GUI.
    $profileName = Get-LastUsedProfile
    if (-not $profileName) { $profileName = [string]$Settings.profileName }
    if ($profileName -and -not (Test-Path (Join-Path $OpenRgbProfileDir ($profileName + '.orp')))) {
        Write-StartupLog "Último perfil '$profileName' não existe mais; verificando o perfil configurado no Control Center."
        $profileName = [string]$Settings.profileName
    }
    if ($profileName -and -not (Test-Path (Join-Path $OpenRgbProfileDir ($profileName + '.orp')))) {
        Write-StartupLog "Perfil configurado '$profileName' não encontrado; iniciando sem perfil."
        $profileName = ''
    }

    $arguments = Get-ProfileArguments -ProfileName $profileName -Minimized ([bool]$Settings.startMinimized)
    $launchStarted = Get-Date
    try {
        $process = Start-Process -FilePath $Exe -ArgumentList $arguments -WorkingDirectory $AppDir -PassThru
        Write-StartupLog ("MasterRGB iniciado. PID=" + $process.Id + "; Perfil='" + $profileName + "'; argumentos=" + $arguments)

        if ($profileName) {
            $confirmed = $false
            for ($attempt = 0; $attempt -lt 30; $attempt++) {
                Start-Sleep -Seconds 2
                if (Test-Path -LiteralPath $LastProfileFile) {
                    $loadedName = (Get-Content -LiteralPath $LastProfileFile -Raw -ErrorAction SilentlyContinue).Trim()
                    $lastWrite = (Get-Item -LiteralPath $LastProfileFile).LastWriteTime
                    if (($loadedName -eq $profileName) -and ($lastWrite -ge $launchStarted.AddSeconds(-1))) {
                        $confirmed = $true
                        break
                    }
                }
                if ($process.HasExited) { break }
            }
            if ($confirmed) {
                Write-StartupLog ("CONFIRMADO: o núcleo carregou o último perfil '" + $profileName + "'.")
            } else {
                Write-StartupLog ("SEM CONFIRMAÇÃO: o perfil '" + $profileName + "' não atualizou o registro de carga. Consulte o log do núcleo em %APPDATA%\OpenRGB.")
            }
        } else {
            Write-StartupLog 'Nenhum perfil salvo anteriormente; MasterRGB iniciou sem perfil.'
        }
    } catch { Write-StartupLog ('Falha ao iniciar: ' + $_.Exception.Message); exit 1 }
    exit 0
}

if (-not (Test-Path -LiteralPath $Exe)) {
    [System.Windows.Forms.MessageBox]::Show("MasterRGB.exe não foi encontrado.`nCompile primeiro ou coloque este inicializador na mesma pasta do executável.",$AppTitle,'OK','Warning')|Out-Null
    exit 1
}

# ---- MasterRGB Control Center ---------------------------------------------
$colors = if ($Settings.theme -eq 'Light') {
    @{ bg=[System.Drawing.Color]::FromArgb(242,245,250); panel=[System.Drawing.Color]::FromArgb(255,255,255); panel2=[System.Drawing.Color]::FromArgb(232,237,246); accent=[System.Drawing.Color]::FromArgb(105,78,210); accent2=[System.Drawing.Color]::FromArgb(34,143,196); text=[System.Drawing.Color]::FromArgb(32,40,58); muted=[System.Drawing.Color]::FromArgb(92,105,127); line=[System.Drawing.Color]::FromArgb(210,218,232) }
} else {
    @{ bg=[System.Drawing.Color]::FromArgb(14,18,28); panel=[System.Drawing.Color]::FromArgb(25,32,47); panel2=[System.Drawing.Color]::FromArgb(32,42,61); accent=[System.Drawing.Color]::FromArgb(133,101,255); accent2=[System.Drawing.Color]::FromArgb(54,195,255); text=[System.Drawing.Color]::FromArgb(238,242,250); muted=[System.Drawing.Color]::FromArgb(157,170,193); line=[System.Drawing.Color]::FromArgb(54,66,91) }
}
$form = New-Object System.Windows.Forms.Form
$form.Text = "MasterRGB $AppVersion"
$form.StartPosition = 'CenterScreen'
$form.Size = New-Object System.Drawing.Size(860, 650)
$form.MinimumSize = New-Object System.Drawing.Size(860, 650)
$form.BackColor = $colors.bg
$form.ForeColor = $colors.text
$form.Font = New-Object System.Drawing.Font('Segoe UI', 9.5)
$form.FormBorderStyle = 'FixedDialog'
$form.MaximizeBox = $false
$iconPath = Join-Path $AppDir 'assets\MasterRGB-icon.ico'
if (Test-Path $iconPath) { try { $form.Icon = New-Object System.Drawing.Icon($iconPath) } catch {} }
function New-Label([string]$text,[int]$x,[int]$y,[int]$w,[int]$h,[int]$size,[bool]$bold,[System.Drawing.Color]$color) {
    $c=New-Object System.Windows.Forms.Label; $c.Text=$text; $c.Location=New-Object System.Drawing.Point($x,$y); $c.Size=New-Object System.Drawing.Size($w,$h)
    $style=if($bold){[System.Drawing.FontStyle]::Bold}else{[System.Drawing.FontStyle]::Regular}; $c.Font=New-Object System.Drawing.Font('Segoe UI',$size,$style); $c.ForeColor=$color; $form.Controls.Add($c); return $c
}
function New-Button([string]$text,[int]$x,[int]$y,[int]$w,[int]$h,[bool]$primary=$false) {
    $c=New-Object System.Windows.Forms.Button; $c.Text=$text; $c.Location=New-Object System.Drawing.Point($x,$y); $c.Size=New-Object System.Drawing.Size($w,$h); $c.FlatStyle='Flat'; $c.FlatAppearance.BorderSize=1
    $c.FlatAppearance.BorderColor=$colors.line; $c.BackColor=$colors.panel2; $c.ForeColor=$colors.text
    if($primary){$c.FlatAppearance.BorderColor=$colors.accent; $c.BackColor=[System.Drawing.Color]::FromArgb(82,62,171)}
    $c.Font=New-Object System.Drawing.Font('Segoe UI Semibold',9,[System.Drawing.FontStyle]::Bold); $form.Controls.Add($c); return $c
}
function New-Card([string]$titleText,[string]$body,[int]$x,[int]$y,[int]$w,[int]$h) {
    $p=New-Object System.Windows.Forms.Panel; $p.Location=New-Object System.Drawing.Point($x,$y); $p.Size=New-Object System.Drawing.Size($w,$h); $p.BackColor=$colors.panel; $p.BorderStyle='FixedSingle'; $form.Controls.Add($p)
    $a=New-Object System.Windows.Forms.Label; $a.Text=$titleText; $a.Location=New-Object System.Drawing.Point(16,13); $a.Size=New-Object System.Drawing.Size(($w-32),24); $a.Font=New-Object System.Drawing.Font('Segoe UI Semibold',11,[System.Drawing.FontStyle]::Bold); $a.ForeColor=$colors.accent2; $p.Controls.Add($a)
    $b=New-Object System.Windows.Forms.Label; $b.Text=$body; $b.Location=New-Object System.Drawing.Point(16,42); $b.Size=New-Object System.Drawing.Size(($w-32),($h-50)); $b.Font=New-Object System.Drawing.Font('Segoe UI',9); $b.ForeColor=$colors.muted; $p.Controls.Add($b); return $p
}
$header=New-Object System.Windows.Forms.Panel; $header.Location=New-Object System.Drawing.Point(0,0); $header.Size=New-Object System.Drawing.Size(860,108); $header.BackColor=$colors.panel2; $form.Controls.Add($header)
$logo=New-Object System.Windows.Forms.Label; $logo.Text='MasterRGB'; $logo.Location=New-Object System.Drawing.Point(30,16); $logo.Size=New-Object System.Drawing.Size(360,45); $logo.Font=New-Object System.Drawing.Font('Segoe UI Semibold',29,[System.Drawing.FontStyle]::Bold); $logo.ForeColor=$colors.text; $header.Controls.Add($logo)
$tag=New-Object System.Windows.Forms.Label; $tag.Text='CONTROLE DE ILUMINAÇÃO DO SEU SETUP'; $tag.Location=New-Object System.Drawing.Point(34,65); $tag.Size=New-Object System.Drawing.Size(500,22); $tag.ForeColor=$colors.muted; $header.Controls.Add($tag)
$version=New-Object System.Windows.Forms.Label; $version.Text='0.2.3 PREVIEW'; $version.Location=New-Object System.Drawing.Point(695,38); $version.Size=New-Object System.Drawing.Size(130,24); $version.TextAlign='MiddleRight'; $version.ForeColor=$colors.accent2; $header.Controls.Add($version)
[void](New-Label 'PAINEL DE CONTROLE' 30 125 300 26 13 $true $colors.text)
[void](New-Label 'Abra o controle RGB, restaure um perfil salvo ou ajuste a inicialização.' 32 154 760 24 9 $false $colors.muted)
[void](New-Card 'MEMÓRIA RGB' 'XPG Spectrix D60G — acesso SMBus pode exigir execução elevada. O perfil é restaurado ao iniciar o editor.' 30 190 245 105)
[void](New-Card 'PLACA DE VÍDEO' 'PowerColor Red Devil RX 6700 XT. Efeitos detectados; ajuste de cores e Meteor continuam em validação.' 307 190 245 105)
[void](New-Card 'PLACA-MÃE' 'MSI MPG B550 Gaming Plus — zonas onboard e conectores RGB/ARGB conforme o suporte do hardware.' 584 190 245 105)
$initialProfile = Get-LastUsedProfile
if (-not $initialProfile) { $initialProfile = [string]$Settings.profileName }
$profileLabel=New-Label ('Último perfil carregado: '+$(if($initialProfile){$initialProfile}else{'Nenhum perfil registrado'})) 32 314 590 24 9 $false $colors.muted
$pick=New-Button 'DEFINIR PERFIL INICIAL (.ORP)' 30 345 240 38
$pick.Add_Click({
    New-Item -ItemType Directory -Force -Path $OpenRgbProfileDir | Out-Null
    $dlg=New-Object System.Windows.Forms.OpenFileDialog; $dlg.Title='Escolher o perfil para usar no próximo início'; $dlg.Filter='Perfis OpenRGB (*.orp)|*.orp'; $dlg.InitialDirectory=$OpenRgbProfileDir; $dlg.Multiselect=$false
    if(Test-Path $OpenRgbProfileDir){$dlg.InitialDirectory=$OpenRgbProfileDir}
    if($dlg.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK){
        try {
            $name=[IO.Path]::GetFileNameWithoutExtension($dlg.FileName)
            $target=Join-Path $OpenRgbProfileDir ($name + '.orp')
            if([IO.Path]::GetFullPath($dlg.FileName) -ne [IO.Path]::GetFullPath($target)){Copy-Item -LiteralPath $dlg.FileName -Destination $target -Force}
            $Settings.profileName=$name; $Settings.profileDisplayName=[IO.Path]::GetFileName($target); Save-Settings $Settings
            $profileLabel.Text='Perfil para inicialização: '+$Settings.profileDisplayName
        } catch { [System.Windows.Forms.MessageBox]::Show($_.Exception.Message,'Falha ao selecionar perfil','OK','Error')|Out-Null }
    }
})
$clear=New-Button 'LIMPAR PERFIL' 260 345 145 38
$clear.Add_Click({
    $Settings.profileName=''; $Settings.profileDisplayName=''; Save-Settings $Settings
    if (Test-Path -LiteralPath $LastProfileFile) { Remove-Item -LiteralPath $LastProfileFile -Force }
    $profileLabel.Text='Último perfil carregado: Nenhum perfil registrado'
})
[void](New-Label 'TEMA' 515 317 100 22 9 $true $colors.muted)
$theme=New-Object System.Windows.Forms.ComboBox; $theme.DropDownStyle='DropDownList'; [void]$theme.Items.Add('Escuro'); [void]$theme.Items.Add('Claro'); $theme.SelectedIndex=0; if($Settings.theme -eq 'Light'){$theme.SelectedIndex=1}; $theme.Location=New-Object System.Drawing.Point(515,345); $theme.Size=New-Object System.Drawing.Size(155,28); $theme.BackColor=$colors.panel2; $theme.ForeColor=$colors.text; $form.Controls.Add($theme)
$save=New-Button 'SALVAR' 681 345 148 38; $save.Add_Click({
    $prevStartup=Get-StartupEnabled; $prevAdmin=[bool]$Settings.runAsAdmin
    $Settings.runAsAdmin=[bool]$admin.Checked; $Settings.startMinimized=[bool]$min.Checked; $Settings.theme=if($theme.SelectedIndex -eq 1){'Light'}else{'Dark'}; Save-Settings $Settings
    $newStartup=[bool]$auto.Checked
    if(($newStartup -ne $prevStartup) -or ($newStartup -and $prevAdmin -ne [bool]$Settings.runAsAdmin)){
        if(Invoke-StartupConfiguration -Enable $newStartup){$Settings.startWithWindows=$newStartup;Save-Settings $Settings;[System.Windows.Forms.MessageBox]::Show('Inicialização do Windows atualizada.','MasterRGB','OK','Information')|Out-Null}
        else {$auto.Checked=Get-StartupEnabled; $Settings.startWithWindows=[bool]$auto.Checked; Save-Settings $Settings}
    } else {$Settings.startWithWindows=$newStartup; Save-Settings $Settings}
    [System.Windows.Forms.MessageBox]::Show('Preferências salvas. O tema será aplicado ao abrir novamente o editor RGB.','MasterRGB','OK','Information')|Out-Null
})
$admin=New-Object System.Windows.Forms.CheckBox; $admin.Text='Executar como administrador (necessário para a RAM XPG nesta configuração)'; $admin.Checked=[bool]$Settings.runAsAdmin; $admin.Location=New-Object System.Drawing.Point(32,403); $admin.Size=New-Object System.Drawing.Size(760,24); $admin.ForeColor=$colors.text; $form.Controls.Add($admin)
$min=New-Object System.Windows.Forms.CheckBox; $min.Text='Iniciar minimizado na bandeja'; $min.Checked=[bool]$Settings.startMinimized; $min.Location=New-Object System.Drawing.Point(32,432); $min.Size=New-Object System.Drawing.Size(340,24); $min.ForeColor=$colors.text; $form.Controls.Add($min)
$auto=New-Object System.Windows.Forms.CheckBox; $auto.Text='Iniciar com o Windows'; $auto.Checked=Get-StartupEnabled; $auto.Location=New-Object System.Drawing.Point(32,461); $auto.Size=New-Object System.Drawing.Size(340,24); $auto.ForeColor=$colors.text; $form.Controls.Add($auto)
$autoNote=New-Label 'A inicialização cria uma tarefa visível no Agendador. No modo portable, mantenha a pasta no mesmo local.' 32 488 790 22 8 $false $colors.muted
$start=New-Button 'ABRIR CONTROLE RGB' 30 536 240 44 $true; $start.Add_Click({$Settings.runAsAdmin=[bool]$admin.Checked;$Settings.startMinimized=[bool]$min.Checked;$Settings.theme=if($theme.SelectedIndex -eq 1){'Light'}else{'Dark'};Save-Settings $Settings;Start-MasterRGB})
$support=New-Button 'SUPORTE / DOAR' 286 536 165 44; $support.Add_Click({Open-Support})
$update=New-Button 'ATUALIZAÇÕES' 467 536 145 44; $update.Add_Click({Check-Updates})
$help=New-Button 'MANUAL' 628 536 90 44; $help.Add_Click({$readme=Join-Path $AppDir 'README-PTBR.md';if(Test-Path $readme){Start-Process $readme}})
$exit=New-Button 'SAIR' 734 536 95 44; $exit.Add_Click({$form.Close()})
[void]$form.ShowDialog()
