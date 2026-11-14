[CmdletBinding()]
param(
    [switch]$Enable,
    [switch]$Disable,
    [Parameter(Mandatory=$false)][string]$ExePath,
    [switch]$RunAsAdmin
)

$ErrorActionPreference = 'Stop'
$TaskName = 'MasterRGB Startup'
$isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if ($Enable -and $Disable) { throw 'Escolha somente -Enable ou -Disable.' }
if (-not $Enable -and -not $Disable) { throw 'Use -Enable ou -Disable.' }

# Registering/removing the task is an explicit, UAC-authorized action.
if (-not $isAdmin) {
    $quotedScript = '"' + $PSCommandPath + '"'
    $argsList = @('-NoProfile','-ExecutionPolicy','Bypass','-File',$quotedScript)
    if ($Enable) { $argsList += '-Enable' } else { $argsList += '-Disable' }
    if ($ExePath) { $argsList += @('-ExePath',('"' + $ExePath + '"')) }
    if ($RunAsAdmin) { $argsList += '-RunAsAdmin' }
    $p = Start-Process -FilePath 'powershell.exe' -Verb RunAs -ArgumentList ($argsList -join ' ') -Wait -PassThru
    exit $p.ExitCode
}

if ($Disable) {
    Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false -ErrorAction SilentlyContinue
    exit 0
}

if (-not $ExePath -or -not (Test-Path -LiteralPath $ExePath)) {
    throw 'MasterRGB.exe não foi encontrado. Informe o caminho completo em -ExePath.'
}

$fullExe = [IO.Path]::GetFullPath($ExePath)
$appDir = Split-Path -Parent $fullExe
$launcher = Join-Path $appDir 'Launch-MasterRGB.ps1'
if (-not (Test-Path -LiteralPath $launcher)) {
    throw 'Launch-MasterRGB.ps1 não foi encontrado junto do executável.'
}

# Supports both installed and portable layouts. For portable, the folder must
# remain at the same path after enabling startup.
$psExe = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'
$arguments = '-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File "' + $launcher + '" -StartupLaunched'
$action = New-ScheduledTaskAction -Execute $psExe -Argument $arguments -WorkingDirectory $appDir
$identity = [Security.Principal.WindowsIdentity]::GetCurrent()
$trigger = New-ScheduledTaskTrigger -AtLogOn -User $identity.Name
$runLevel = if ($RunAsAdmin) { 'Highest' } else { 'Limited' }
$principal = New-ScheduledTaskPrincipal -UserId $identity.Name -LogonType Interactive -RunLevel $runLevel
$settings = New-ScheduledTaskSettingsSet -StartWhenAvailable -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -MultipleInstances IgnoreNew
$description = if ($RunAsAdmin) {
    'Inicia MasterRGB com o perfil escolhido e privilégios elevados autorizados pelo usuário.'
} else {
    'Inicia MasterRGB com o perfil escolhido sem elevação administrativa.'
}
Register-ScheduledTask -TaskName $TaskName -Action $action -Trigger $trigger -Principal $principal -Settings $settings -Description $description -Force | Out-Null
exit 0
