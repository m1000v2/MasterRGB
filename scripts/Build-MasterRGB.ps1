[CmdletBinding()]
param(
    [string]$Workspace = (Join-Path $env:USERPROFILE 'Documents\MasterRGB-Build'),
    [switch]$BuildOnly
)

$ErrorActionPreference = 'Stop'
$SourceUrl = 'https://gitlab.com/CalcProgrammer1/OpenRGB.git'
$SourceBranch = 'release_candidate_1.0rc2wr0'
$QtRoot = if ($env:MASTER_RGB_QT_ROOT) { $env:MASTER_RGB_QT_ROOT } else { 'C:\Qt\6.8.3\msvc2022_64' }
$QtBin = Join-Path $QtRoot 'bin'
$JomExeFromEnv = $env:MASTER_RGB_JOM
$JomBin = 'C:\Qt\jom'

function Require-Command([string]$Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Comando obrigatório não encontrado: $Name. Instale a ferramenta e execute novamente."
    }
}

Require-Command 'git'

$sourceDir = Join-Path $Workspace 'OpenRGB'
New-Item -ItemType Directory -Force -Path $Workspace | Out-Null

if (-not (Test-Path (Join-Path $sourceDir '.git'))) {
    if (Test-Path $sourceDir) {
        throw "A pasta '$sourceDir' já existe e não é um clone Git. Renomeie-a ou escolha outro -Workspace."
    }
    Write-Host "Baixando a base WinRing0 testada, branch $SourceBranch..." -ForegroundColor Cyan
    git clone --depth 1 --branch $SourceBranch $SourceUrl $sourceDir
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao clonar a base OpenRGB.' }
}

# Rebuild from the pristine selected branch for the UI source files only.
# This removes the previous 0.2.2 branding injection without touching the
# PowerColor detector file, so the RX 6700 XT registration remains preserved.
Write-Host 'Preparando uma árvore limpa dos arquivos de interface/branding...' -ForegroundColor Cyan
& git -C $sourceDir checkout -- qt/OpenRGBDialog/OpenRGBDialog.cpp cli.cpp OpenRGB.pro
if ($LASTEXITCODE -ne 0) { throw 'Não foi possível restaurar os arquivos-base da interface. Confira o clone Git.' }

$detector = Join-Path $sourceDir 'Controllers\PowerColorGPUController\PowerColorGPUControllerDetect.cpp'
$pciIds = Join-Path $sourceDir 'pci_ids\pci_ids.h'
if (-not (Test-Path $detector)) { throw "Arquivo de detecção não encontrado: $detector" }
if (-not (Test-Path $pciIds)) { throw "Arquivo de IDs PCI não encontrado: $pciIds" }

$idsText = Get-Content -Raw -Path $pciIds
if ($idsText -notmatch '#define\s+POWERCOLOR_RED_DEVIL_RX6700XT_SUB_DEV\s+0x2409') {
    throw 'A base selecionada não contém o ID 148C:2409 da Red Devil RX 6700 XT. Não vou aplicar uma alteração incompatível.'
}

$sourceText = Get-Content -Raw -Path $detector

# Brand the produced executable while preserving OpenRGB's internal compatibility paths.
$projectFile = Join-Path $sourceDir 'OpenRGB.pro'
$projectText = Get-Content -Raw -Path $projectFile
if ($projectText -match '(?m)^TARGET\s*=\s*OpenRGB\s*$') {
    $projectText = [regex]::Replace($projectText, '(?m)^TARGET\s*=\s*OpenRGB\s*$', 'TARGET = MasterRGB', 1)
    [System.IO.File]::WriteAllText($projectFile, $projectText, [System.Text.UTF8Encoding]::new($false))
}
if ($sourceText -notmatch 'PowerColor Red Devil RX6700XT') {
    $newLine = 'REGISTER_I2C_PCI_DETECTOR("PowerColor Red Devil RX6700XT",          DetectPowerColorRedDevilGPUControllersV1, AMD_GPU_VEN, AMD_NAVI22_DEV, POWERCOLOR_SUB_VEN, POWERCOLOR_RED_DEVIL_RX6700XT_SUB_DEV,              0x22);'
    $pattern = '(?m)^(?<existing>REGISTER_I2C_PCI_DETECTOR\("PowerColor Red Devil RX5700XT"[^\r\n]*\r?\n)'
    if (-not [regex]::IsMatch($sourceText, $pattern)) {
        throw 'Não encontrei o ponto seguro para inserir o registro da RX 6700 XT. Nenhum arquivo foi alterado.'
    }
    $sourceText = [regex]::Replace($sourceText, $pattern, ('$1' + $newLine + "`r`n"), 1)
    [System.IO.File]::WriteAllText($detector, $sourceText, [System.Text.UTF8Encoding]::new($false))
    Write-Host 'Registro da PowerColor Red Devil RX 6700 XT aplicado.' -ForegroundColor Green
} else {
    Write-Host 'Registro da RX 6700 XT já existe; mantendo o arquivo.' -ForegroundColor Yellow
}

# Add app branding and bundled dark/light theme loading without requiring Skin Plugin.
$brandingScript = Join-Path $PSScriptRoot '..\patches\Apply-MasterRGBBranding.ps1'
& $brandingScript -SourceDir $sourceDir
$lastProfileScript = Join-Path $PSScriptRoot '..\patches\Apply-MasterRGBLastProfile.ps1'
& $lastProfileScript -SourceDir $sourceDir

if ($BuildOnly) {
    Write-Host "Código preparado em: $sourceDir" -ForegroundColor Green
    exit 0
}

$qmake = Join-Path $QtBin 'qmake.exe'
$windeployqt = Join-Path $QtBin 'windeployqt.exe'
foreach ($path in @($qmake, $windeployqt)) {
    if (-not (Test-Path $path)) {
        throw "Dependência não encontrada: $path`nConfira o Qt 6.8.3 MSVC 2022 x64. Consulte README-PTBR.md."
    }
}

# Locate Visual Studio in both traditional and newer install roots. This also
# supports the user's VS 2026 Community installation under Program Files\...\18.
$vsPath = $null
$vcvars = $null
$vswhereCandidates = @(
    (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\vswhere.exe'),
    (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe')
)
foreach ($candidate in $vswhereCandidates) {
    if (Test-Path $candidate) {
        $found = & $candidate -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($found) {
            $candidateVcvars = Join-Path $found 'VC\Auxiliary\Build\vcvarsall.bat'
            if (-not (Test-Path $candidateVcvars)) { $candidateVcvars = Join-Path $found 'VC\Auxiliary\Build\vcvars64.bat' }
            if (Test-Path $candidateVcvars) { $vsPath = $found; $vcvars = $candidateVcvars; break }
        }
    }
}

# Fallback for VS 2026 Community where vswhere may not be in its usual location.
if (-not $vcvars) {
    $fallbackRoots = @(
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\18\Community'),
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\2022\Community'),
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\2022\BuildTools'),
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\2022\Community'),
        (Join-Path $env:ProgramFiles 'Microsoft Visual Studio\2022\BuildTools')
    )
    foreach ($root in $fallbackRoots) {
        $candidateVcvars = Join-Path $root 'VC\Auxiliary\Build\vcvarsall.bat'
        if (-not (Test-Path $candidateVcvars)) { $candidateVcvars = Join-Path $root 'VC\Auxiliary\Build\vcvars64.bat' }
        if (Test-Path $candidateVcvars) { $vsPath = $root; $vcvars = $candidateVcvars; break }
    }
}
if (-not $vcvars) {
    throw 'Não encontrei vcvarsall.bat. Instale o componente Desktop development with C++ no Visual Studio.'
}

# Qt 6.8.3 msvc2022_64 needs the MSVC v143 toolset. On the detected VS 2026
# machine, 14.41 is installed alongside 14.51; deliberately select 14.41.
$msvcRoot = Join-Path $vsPath 'VC\Tools\MSVC'
$toolsets = @(Get-ChildItem $msvcRoot -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending)
$toolset = $toolsets | Where-Object { $_.Name -like '14.41.*' } | Select-Object -First 1
if (-not $toolset) {
    $toolset = $toolsets | Where-Object { $_.Name -match '^14\.4[0-9]\.' } | Select-Object -First 1
}
if (-not $toolset) {
    throw "Não encontrei um toolset MSVC v143 compatível (14.4x) em $msvcRoot. O Qt instalado é msvc2022_64 e precisa do toolset v143."
}
$vcvarsVersion = ($toolset.Name -replace '^(\d+\.\d+)\..*','$1')
$clPath = Join-Path $toolset.FullName 'bin\Hostx64\x64\cl.exe'
if (-not (Test-Path $clPath)) { throw "cl.exe não encontrado: $clPath" }

# Prefer the same JOM tool used by the local developer build. On clean GitHub
# Windows runners, fall back to Microsoft's NMAKE shipped with the selected
# MSVC toolset; this avoids relying on an unverified Chocolatey JOM package.
$jom = if ($JomExeFromEnv -and (Test-Path $JomExeFromEnv)) {
    $JomExeFromEnv
} elseif (Get-Command jom.exe -ErrorAction SilentlyContinue) {
    (Get-Command jom.exe).Source
} elseif (Test-Path (Join-Path $JomBin 'jom.exe')) {
    Join-Path $JomBin 'jom.exe'
} else { $null }
if ($jom) {
    $makeDir = Split-Path -Parent $jom
    $makeLine = 'jom'
    $makeLabel = 'JOM'
} else {
    $nmake = Join-Path (Split-Path -Parent $clPath) 'nmake.exe'
    if (-not (Test-Path $nmake)) { throw "JOM e NMAKE não encontrados. Esperado NMAKE em $nmake" }
    $makeDir = Split-Path -Parent $nmake
    $makeLine = 'nmake /nologo'
    $makeLabel = 'NMAKE'
}

$cmdFile = Join-Path $Workspace 'build-masterrgb.cmd'
$cmd = @"
@echo off
if /I "$([IO.Path]::GetFileName($vcvars))"=="vcvars64.bat" (call "$vcvars" -vcvars_ver=$vcvarsVersion) else (call "$vcvars" x64 -vcvars_ver=$vcvarsVersion)
if errorlevel 1 exit /b %errorlevel%
set "PATH=$QtBin;$makeDir;%PATH%"
cd /d "$sourceDir"
qmake OpenRGB.pro CONFIG-=debug_and_release CONFIG+=release
if errorlevel 1 exit /b %errorlevel%
$makeLine
if errorlevel 1 exit /b %errorlevel%
windeployqt --no-patchqt --no-quick-import --no-translations --no-system-d3d-compiler --no-opengl-sw .\release\MasterRGB.exe
if errorlevel 1 exit /b %errorlevel%
exit /b 0
"@
[System.IO.File]::WriteAllText($cmdFile, $cmd, [System.Text.Encoding]::ASCII)

Write-Host "Visual Studio: $vsPath" -ForegroundColor DarkGray
Write-Host "Toolset MSVC selecionado: $vcvarsVersion (v143)" -ForegroundColor DarkGray
Write-Host "Make selecionado: $makeLabel" -ForegroundColor DarkGray
Write-Host 'Compilando MasterRGB Core...' -ForegroundColor Cyan
& $env:ComSpec /d /c $cmdFile
if ($LASTEXITCODE -ne 0) { throw "Compilação falhou com código $LASTEXITCODE. Consulte a saída acima." }

$releaseDir = Join-Path $sourceDir 'release'
$builtExe = Join-Path $releaseDir 'MasterRGB.exe'
if (-not (Test-Path $builtExe)) { throw "MasterRGB.exe não foi gerado em $releaseDir" }

$distRoot = Join-Path $Workspace 'dist'
New-Item -ItemType Directory -Force -Path $distRoot | Out-Null
$portableDir = Join-Path $distRoot 'MasterRGB-Portable'
if (Test-Path $portableDir) { Remove-Item $portableDir -Recurse -Force }
New-Item -ItemType Directory -Force -Path $portableDir | Out-Null
Copy-Item (Join-Path $releaseDir '*') $portableDir -Recurse -Force
New-Item -ItemType Directory -Force -Path (Join-Path $portableDir 'themes') | Out-Null
Copy-Item (Join-Path $PSScriptRoot '..\themes\*.qss') (Join-Path $portableDir 'themes') -Force
New-Item -ItemType Directory -Force -Path (Join-Path $portableDir 'config') | Out-Null
Copy-Item (Join-Path $PSScriptRoot '..\config\*') (Join-Path $portableDir 'config') -Recurse -Force
New-Item -ItemType Directory -Force -Path (Join-Path $portableDir 'assets') | Out-Null
Copy-Item (Join-Path $PSScriptRoot '..\assets\*') (Join-Path $portableDir 'assets') -Recurse -Force
Copy-Item (Join-Path $PSScriptRoot 'Launch-MasterRGB.ps1') $portableDir -Force
Copy-Item (Join-Path $PSScriptRoot 'Set-MasterRGBStartup.ps1') $portableDir -Force
Copy-Item (Join-Path $PSScriptRoot 'Start-MasterRGB.cmd') $portableDir -Force
Copy-Item (Join-Path $PSScriptRoot '..\README-PTBR.md') $portableDir -Force
Copy-Item (Join-Path $PSScriptRoot '..\LICENSE') $portableDir -Force
Copy-Item (Join-Path $PSScriptRoot '..\NOTICE.md') $portableDir -Force
Copy-Item (Join-Path $PSScriptRoot '..\CHANGELOG.md') $portableDir -Force

$zipPath = Join-Path $distRoot 'MasterRGB-0.2.3-Preview-Portable.zip'
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
Compress-Archive -Path (Join-Path $portableDir '*') -DestinationPath $zipPath -CompressionLevel Optimal

# Mirror the portable payload beside installer\MasterRGB.iss so Inno Setup's
# relative [Files] source resolves regardless of the selected build workspace.
$kitRoot = Split-Path -Parent $PSScriptRoot
$kitDist = Join-Path $kitRoot 'dist'
$kitPortable = Join-Path $kitDist 'MasterRGB-Portable'
New-Item -ItemType Directory -Force -Path $kitDist | Out-Null
if (Test-Path $kitPortable) { Remove-Item $kitPortable -Recurse -Force }
Copy-Item $portableDir $kitPortable -Recurse -Force
Copy-Item $zipPath $kitDist -Force

# Inno Setup compilation is separate and uses the mirrored kit\dist folder.
Write-Host ''
Write-Host 'Build MasterRGB 0.2.3 Preview concluído.' -ForegroundColor Green
Write-Host "Executável: $(Join-Path $portableDir 'MasterRGB.exe')"
Write-Host "Portable:   $zipPath"
Write-Host 'Para gerar o instalador, compile installer\MasterRGB.iss com o Inno Setup 6.'
Write-Host 'Os temas MasterRGB são carregados pelo próprio executável; o seletor também fica em Configurações > Aparência.'
