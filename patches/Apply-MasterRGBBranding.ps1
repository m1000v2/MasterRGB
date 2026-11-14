[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$SourceDir
)

$ErrorActionPreference = 'Stop'
$dialog = Join-Path $SourceDir 'qt\OpenRGBDialog\OpenRGBDialog.cpp'
if (-not (Test-Path -LiteralPath $dialog)) {
    throw "Não encontrei o arquivo de interface Qt esperado: $dialog"
}

$marker = 'MASTER_RGB_UI_030'

# The build script restores the upstream copies before each run. Refuse to
# stack a second block if someone re-runs this script on the same tree.
$text = [IO.File]::ReadAllText($dialog)
if ($text.Contains($marker)) {
    Write-Host 'Interface MasterRGB 0.3.0 já aplicada; nada a fazer.' -ForegroundColor Yellow
    exit 0
}

#-----------------------------------------------------------------------------------------------
# 1. Copy the real MasterRGB source files into the Qt source tree.
#    OpenRGB.pro globs qt/*.cpp and qt/*.h, so no project edit is required.
#-----------------------------------------------------------------------------------------------
$sourceFiles = @('MasterRGBTheme.h', 'MasterRGBTheme.cpp', 'MasterRGBUi.h', 'MasterRGBUi.cpp')
foreach ($name in $sourceFiles) {
    $from = Join-Path $PSScriptRoot "source\qt\$name"
    $to = Join-Path $SourceDir "qt\$name"
    if (-not (Test-Path -LiteralPath $from)) {
        throw "Arquivo de origem ausente: $from"
    }
    Copy-Item -LiteralPath $from -Destination $to -Force
}

#-----------------------------------------------------------------------------------------------
# 2. OpenRGBDialog.cpp: include and attach the MasterRGB pages after the
#    upstream theme manager initializes. All page construction lives in
#    MasterRGBUi.cpp, not in an injected block.
#-----------------------------------------------------------------------------------------------
$includeLine = '#include "MasterRGBUi.h"'
if (-not $text.Contains($includeLine)) {
    $firstInclude = [regex]::Match($text, '(?m)^#include [^\r\n]+')
    if (-not $firstInclude.Success) { throw 'Não encontrei a área de includes em OpenRGBDialog.cpp.' }
    $text = $text.Insert($firstInclude.Index + $firstInclude.Length, "`r`n$includeLine")
}

$needle = 'OpenRGBThemeManager::Init();'
$insertAt = $text.IndexOf($needle, [StringComparison]::Ordinal)
if ($insertAt -lt 0) {
    throw 'Não encontrei OpenRGBThemeManager::Init(); na base esperada. Nenhuma alteração foi aplicada.'
}
$insertAt += $needle.Length
$attach = @"

    // MASTER_RGB_UI_030 — MasterRGB pages, theme and branding.
    MasterRGBUi::Attach(this);
"@
$text = $text.Insert($insertAt, $attach)

# Preserve upstream attribution but use the product name in the tray tooltip.
$text = $text.Replace('trayIcon->setToolTip("OpenRGB");', 'trayIcon->setToolTip("MasterRGB");')
[IO.File]::WriteAllText($dialog, $text, [Text.UTF8Encoding]::new($false))

#-----------------------------------------------------------------------------------------------
# 3. General Settings: the existing theme combo now applies live and reads
#    the MasterRGB preference. The separate "Aparência" tab is gone.
#-----------------------------------------------------------------------------------------------
$settingsCpp = Join-Path $SourceDir 'qt\OpenRGBSettingsPage\OpenRGBSettingsPage.cpp'
$settingsText = [IO.File]::ReadAllText($settingsCpp)

if (-not $settingsText.Contains('#include "../MasterRGBTheme.h"')) {
    $settingsText = $settingsText.Replace('#include "ui_OpenRGBSettingsPage.h"',
        "#include `"ui_OpenRGBSettingsPage.h`"`r`n#include `"../MasterRGBTheme.h`"")
}

$themeInitPattern = '(?s)    ui->ComboBoxTheme->addItems\(\{"Auto", "Light", "Dark"\}\);\s*json theme_settings = ResourceManager::get\(\)->GetSettingsManager\(\)->GetSettings\("Theme"\);\s*if\(theme_settings\.contains\("theme"\)\)\s*\{.*?\}\s*else\s*\{.*?\}\s*theme_initialized = true;'
$themeInitReplacement = @'
    ui->ComboBoxTheme->addItems({"Auto", "Light", "Dark"});

    /*---------------------------------------------------------*\
    | MASTER_RGB_THEME_INIT: read the MasterRGB preference      |
    \*---------------------------------------------------------*/
    ui->ComboBoxTheme->setCurrentText(MasterRGBTheme::Current());
    ui->ThemeLabel->setText("Tema da interface (aplica na hora)");

    theme_initialized = true;
'@
if ([regex]::IsMatch($settingsText, $themeInitPattern)) {
    $settingsText = [regex]::Replace($settingsText, $themeInitPattern, [System.Text.RegularExpressions.MatchEvaluator]{ param($m) $themeInitReplacement }, 1)
}

$themeHandlerPattern = '(?s)void OpenRGBSettingsPage::on_ComboBoxTheme_currentTextChanged\(const QString theme\)\s*\{.*?\n\}'
$themeHandlerReplacement = @'
void OpenRGBSettingsPage::on_ComboBoxTheme_currentTextChanged(const QString theme)
{
    if(theme_initialized)
    {
        json theme_settings = ResourceManager::get()->GetSettingsManager()->GetSettings("Theme");
        theme_settings["theme"] = theme.toStdString();
        ResourceManager::get()->GetSettingsManager()->SetSettings("Theme", theme_settings);
        SaveSettings();

        /* MASTER_RGB_THEME_LIVE: apply the palette and QSS immediately. */
        MasterRGBTheme::Save(theme);
        MasterRGBTheme::Apply(theme);
    }
}
'@
if ([regex]::IsMatch($settingsText, $themeHandlerPattern)) {
    $settingsText = [regex]::Replace($settingsText, $themeHandlerPattern, [System.Text.RegularExpressions.MatchEvaluator]{ param($param) $themeHandlerReplacement }, 1)
}

# Use a MasterRGB-named Startup shortcut instead of OpenRGB.lnk so the entry
# is clearly ours in Task Manager > Startup apps.
$settingsText = $settingsText.Replace('AutoStart auto_start("OpenRGB");', 'AutoStart auto_start("MasterRGB");')

# If the installer created the Startup shortcut, show the in-app option as on.
$settingsText = $settingsText.Replace(
    'ui->CheckboxAutoStart->setChecked(autostart_settings["enabled"]);',
    'ui->CheckboxAutoStart->setChecked(autostart_settings["enabled"] || AutoStart("MasterRGB").IsAutoStartEnabled());')
$settingsText = $settingsText.Replace(
    'SetAutoStartVisibility(autostart_settings["enabled"]);',
    'SetAutoStartVisibility(ui->CheckboxAutoStart->isChecked());')

[IO.File]::WriteAllText($settingsCpp, $settingsText, [Text.UTF8Encoding]::new($false))

#-----------------------------------------------------------------------------------------------
# 4. About page: real product version, project link and values that survive
#    a language change (retranslateUi resets the .ui placeholder strings).
#-----------------------------------------------------------------------------------------------
$infoCpp = Join-Path $SourceDir 'qt\OpenRGBSoftwareInfoPage\OpenRGBSoftwareInfoPage.cpp'
$infoText = [IO.File]::ReadAllText($infoCpp)
$infoText = $infoText.Replace("`r`n", "`n")

$infoText = $infoText.Replace('    ui->VersionValue->setText(VERSION_STRING);',
    '    ui->VersionValue->setText("MasterRGB 0.3.0 (base OpenRGB 1.0rc2wr0)");')

if (-not $infoText.Contains('MASTER_RGB_SITE_LINK')) {
    $infoText = $infoText.Replace(
        '    ui->OsVersionValue->setText(QSysInfo::prettyProductName());',
        @'
    ui->OsVersionValue->setText(QSysInfo::prettyProductName());
    /* MASTER_RGB_SITE_LINK */
    ui->WebsiteLinkValue->setText("<a href=\"https://github.com/m1000v2/MasterRGB\">github.com/m1000v2/MasterRGB</a>");
'@)
}

if (-not $infoText.Contains('MASTER_RGB_RETRANSLATE_VALUES')) {
    $infoText = $infoText.Replace(
        @'
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
'@,
        @'
    if(event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        /* MASTER_RGB_RETRANSLATE_VALUES: restore dynamic values after the
           .ui placeholders are re-applied by retranslateUi. */
        ui->QtVersionValue->setText(QT_VERSION_STR);
        ui->OsVersionValue->setText(QSysInfo::prettyProductName());
        ui->VersionValue->setText("MasterRGB 0.3.0 (base OpenRGB 1.0rc2wr0)");
        ui->WebsiteLinkValue->setText("<a href=\"https://github.com/m1000v2/MasterRGB\">github.com/m1000v2/MasterRGB</a>");
    }
'@)
}

[IO.File]::WriteAllText($infoCpp, $infoText, [Text.UTF8Encoding]::new($false))

#-----------------------------------------------------------------------------------------------
# 5. CLI product name.
#-----------------------------------------------------------------------------------------------
$cli = Join-Path $SourceDir 'cli.cpp'
if (Test-Path -LiteralPath $cli) {
    $cliText = [IO.File]::ReadAllText($cli)
    $cliText = $cliText.Replace('help_text += "OpenRGB ";', 'help_text += "MasterRGB ";')
    $cliText = $cliText.Replace('version_text += "OpenRGB ";', 'version_text += "MasterRGB ";')
    $cliText = $cliText.Replace('Run `OpenRGB --help` for syntax', 'Run `MasterRGB --help` for syntax')
    $cliText = $cliText.Replace('Usage: OpenRGB (', 'Usage: MasterRGB (')
    [IO.File]::WriteAllText($cli, $cliText, [Text.UTF8Encoding]::new($false))
}

Write-Host 'Interface MasterRGB 0.3.0 aplicada (páginas nativas, tema ao vivo, Apoiar e Sobre).' -ForegroundColor Green
