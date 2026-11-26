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

$marker = 'MASTER_RGB_UI_040'
$Version = '0.4.0'
$versionHeader = Join-Path $PSScriptRoot 'source\qt\MasterRGBVersion.h'
if (Test-Path -LiteralPath $versionHeader) {
    $versionText = Get-Content -Raw -LiteralPath $versionHeader
    $versionMatch = [regex]::Match($versionText, '#define\s+MASTERRGB_VERSION\s+"([^"]+)"')
    if ($versionMatch.Success) { $Version = $versionMatch.Groups[1].Value }
}

# The build script restores the upstream copies before each run. Refuse to
# stack a second block if someone re-runs this script on the same tree.
$text = [IO.File]::ReadAllText($dialog)
if ($text.Contains($marker)) {
    Write-Host 'Interface MasterRGB 0.4.0 já aplicada; nada a fazer.' -ForegroundColor Yellow
    exit 0
}

#-----------------------------------------------------------------------------------------------
# 1. Copy the real MasterRGB source files into the Qt source tree.
#    OpenRGB.pro globs qt/*.cpp and qt/*.h, so no project edit is required.
#-----------------------------------------------------------------------------------------------
$sourceFiles = @('MasterRGBTheme.h', 'MasterRGBTheme.cpp', 'MasterRGBUi.h', 'MasterRGBUi.cpp',
                 'MasterRGBVersion.h', 'MasterRGBUpdate.h', 'MasterRGBUpdate.cpp')
foreach ($name in $sourceFiles) {
    $from = Join-Path $PSScriptRoot "source\qt\$name"
    $to = Join-Path $SourceDir "qt\$name"
    if (-not (Test-Path -LiteralPath $from)) {
        throw "Arquivo de origem ausente: $from"
    }
    Copy-Item -LiteralPath $from -Destination $to -Force
}

$pluginFiles = @('OpenRGBPluginInfo.h', 'OpenRGBPluginInterface.h', 'PluginManager.h', 'PluginManager.cpp',
                 'MasterRGBPluginBridge.h', 'MasterRGBPluginBridge.cpp', 'OpenRGBPluginAPI.h', 'OpenRGBPluginAPI.cpp')
foreach ($name in $pluginFiles) {
    $from = Join-Path $PSScriptRoot "source\$name"
    $to = Join-Path $SourceDir $name
    if (Test-Path -LiteralPath $from) {
        Copy-Item -LiteralPath $from -Destination $to -Force
    }
}

$controllerFiles = @('RGBControllerAccess.h', 'RGBControllerAccess.cpp', 'RGBControllerAdapter.h',
                     'RGBControllerAdapter.cpp', 'RGBControllerInterface.h', 'RGBController_Virtual.h',
                     'RGBController_Virtual.cpp')
foreach ($name in $controllerFiles) {
    $from = Join-Path $PSScriptRoot "source\$name"
    $to = Join-Path $SourceDir "RGBController\$name"
    if (Test-Path -LiteralPath $from) {
        Copy-Item -LiteralPath $from -Destination $to -Force
    }
}

#-----------------------------------------------------------------------------------------------
# 1b. Product icons and executable version info.
#     qmake builds its Windows .rc from RC_ICONS + VERSION, so replacing the
#     upstream icon file is what makes the shortcut, taskbar, Alt+Tab and the
#     file properties show the MasterRGB identity instead of OpenRGB.
#-----------------------------------------------------------------------------------------------
$kitRoot = Split-Path -Parent $PSScriptRoot
$iconIco = Join-Path $kitRoot 'assets\MasterRGB-icon.ico'
$iconPng = Join-Path $kitRoot 'assets\MasterRGB-icon.png'
if (Test-Path -LiteralPath $iconIco) {
    Copy-Item -LiteralPath $iconIco -Destination (Join-Path $SourceDir 'qt\OpenRGB.ico') -Force
}
if (Test-Path -LiteralPath $iconPng) {
    Copy-Item -LiteralPath $iconPng -Destination (Join-Path $SourceDir 'qt\org.openrgb.OpenRGB.png') -Force
}

$projectFile = Join-Path $SourceDir 'OpenRGB.pro'
$projectText = [IO.File]::ReadAllText($projectFile)
if ($projectText -notmatch '(?m)^QT\s*\+=\s*network\s*$') {
    $projectText = $projectText.Replace('greaterThan(QT_MAJOR_VERSION, 4): QT += widgets',
        "greaterThan(QT_MAJOR_VERSION, 4): QT += widgets`r`nQT += network")
}
if ($projectText -notmatch '(?m)^VERSION\s*=') {
    $projectText = [regex]::Replace($projectText, '(?m)^TARGET\s*=\s*MasterRGB\s*$',
        "TARGET = MasterRGB`r`nVERSION = $Version", 1)
}

$headersPattern = '(?m)^[ \t]*OpenRGBPluginInterface\.h[ \t]*\\[\r\n]+[ \t]*PluginManager\.h[ \t]*\\'
if ($projectText -match $headersPattern) {
    $pluginHeadersBlock = @"
    OpenRGBPluginInfo.h                                                                         \
    OpenRGBPluginInterface.h                                                                    \
    PluginManager.h                                                                             \
    MasterRGBPluginBridge.h                                                                     \
    OpenRGBPluginAPI.h                                                                          \
    RGBController/RGBControllerAccess.h                                                         \
    RGBController/RGBControllerAdapter.h                                                        \
    RGBController/RGBControllerInterface.h                                                      \
    RGBController/RGBController_Virtual.h                                                       \
"@
    $projectText = [regex]::Replace($projectText, $headersPattern, $pluginHeadersBlock)
}

$sourcesPattern = '(?m)^[ \t]*PluginManager\.cpp[ \t]*\\\r?$'
if ($projectText -match $sourcesPattern -and $projectText -notmatch 'MasterRGBPluginBridge\.cpp') {
    $pluginSourcesBlock = @"
    PluginManager.cpp                                                                           \
    MasterRGBPluginBridge.cpp                                                                   \
    OpenRGBPluginAPI.cpp                                                                        \
    RGBController/RGBControllerAccess.cpp                                                       \
    RGBController/RGBControllerAdapter.cpp                                                      \
    RGBController/RGBController_Virtual.cpp                                                     \
"@
    $projectText = [regex]::Replace($projectText, $sourcesPattern, $pluginSourcesBlock)
}
[IO.File]::WriteAllText($projectFile, $projectText, [Text.UTF8Encoding]::new($false))

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
$text = $text.Replace('plugin->plugin->GetWidget()', 'plugin->widget')
$text = $text.Replace('plugin->plugin->GetTrayMenu()', 'plugin->traymenu')
$text = $text.Replace("case OPENRGB_PLUGIN_LOCATION_SETTINGS:
            plugin_parent_widget = ui->InformationTabBar;", "case OPENRGB_PLUGIN_LOCATION_SETTINGS:
            plugin_parent_widget = ui->SettingsTabBar;")
$text = $text.Replace("case OPENRGB_PLUGIN_LOCATION_SETTINGS:
            plugin_parent_widget = ui->InformationTabBar;", "case OPENRGB_PLUGIN_LOCATION_SETTINGS:
            plugin_parent_widget = ui->SettingsTabBar;")
$text = [regex]::Replace($text, 'void OpenRGBDialog::AddPlugin\(OpenRGBPluginEntry\* plugin\)\s*\{', "void OpenRGBDialog::AddPlugin(OpenRGBPluginEntry* plugin)
{
    if(plugin == nullptr) { return; }")
$text = [regex]::Replace($text, 'void OpenRGBDialog::RemovePlugin\(OpenRGBPluginEntry\* plugin\)\s*\{', "void OpenRGBDialog::RemovePlugin(OpenRGBPluginEntry* plugin)
{
    if(plugin == nullptr) { return; }")
[IO.File]::WriteAllText($dialog, $text, [Text.UTF8Encoding]::new($false))

$containerFile = Join-Path $SourceDir 'qt\OpenRGBPluginContainer\OpenRGBPluginContainer.cpp'
if (Test-Path -LiteralPath $containerFile) {
    $containerText = [IO.File]::ReadAllText($containerFile)
    $containerText = [regex]::Replace($containerText, 'plugin_widget = plugin;\s*plugin_widget->setParent\(this\);\s*ui->PluginContainerLayout->layout\(\)->addWidget\(plugin_widget\);\s*Hide\(\);', "plugin_widget = plugin;
    if(plugin_widget != nullptr)
    {
        plugin_widget->setParent(this);
        ui->PluginContainerLayout->layout()->addWidget(plugin_widget);
        Hide();
    }")
    [IO.File]::WriteAllText($containerFile, $containerText, [Text.UTF8Encoding]::new($false))
}

#-----------------------------------------------------------------------------------------------
# 2b. Visible strings that still said OpenRGB: window title, startup message,
#     About links, "GitLab" label and the product description. Attribution to
#     the OpenRGB authors stays in the About page and in NOTICE.md (GPL-2.0).
#-----------------------------------------------------------------------------------------------
$dialogUi = Join-Path $SourceDir 'qt\OpenRGBDialog\OpenRGBDialog.ui'
if (Test-Path -LiteralPath $dialogUi) {
    $dialogUiText = [IO.File]::ReadAllText($dialogUi)
    $dialogUiText = $dialogUiText.Replace('<string>OpenRGB</string>', '<string>MasterRGB</string>')
    $dialogUiText = $dialogUiText.Replace('<string>OpenRGB is detecting devices...</string>',
        '<string>O MasterRGB esta detectando os dispositivos...</string>')
    [IO.File]::WriteAllText($dialogUi, $dialogUiText, [Text.UTF8Encoding]::new($false))
}

$infoUi = Join-Path $SourceDir 'qt\OpenRGBSoftwareInfoPage\OpenRGBSoftwareInfoPage.ui'
if (Test-Path -LiteralPath $infoUi) {
    $infoUiText = [IO.File]::ReadAllText($infoUi)
    $infoUiText = $infoUiText.Replace('https://gitlab.com/CalcProgrammer1/OpenRGB', 'https://github.com/m1000v2/MasterRGB')
    $infoUiText = $infoUiText.Replace('<string>GitLab:</string>', '<string>GitHub:</string>')
    $infoUiText = $infoUiText.Replace('https://openrgb.org', 'https://github.com/m1000v2/MasterRGB/releases')
    $infoUiText = $infoUiText.Replace('<string>Adam Honse, OpenRGB Team</string>',
        '<string>Adam Honse (base OpenRGB) e projeto MasterRGB</string>')
    $infoUiText = $infoUiText.Replace('&lt;b&gt;OpenRGB&lt;/b&gt;, an open-source RGB control utility',
        '&lt;b&gt;MasterRGB&lt;/b&gt;, controle de iluminacao RGB para Windows')
    [IO.File]::WriteAllText($infoUi, $infoUiText, [Text.UTF8Encoding]::new($false))
}

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
    "    ui->VersionValue->setText(`"MasterRGB $Version (base OpenRGB 1.0rc2wr0)`");")

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

$infoText = $infoText.Replace('MasterRGB 0.3.0 (base OpenRGB 1.0rc2wr0)', "MasterRGB $Version (base OpenRGB 1.0rc2wr0)")
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

#-----------------------------------------------------------------------------------------------
# 6. Default destructors for mode and zone to avoid LNK2005 with plugin API.
#-----------------------------------------------------------------------------------------------
$controllerH = Join-Path $SourceDir 'RGBController\RGBController.h'
if (Test-Path -LiteralPath $controllerH) {
    $textH = [IO.File]::ReadAllText($controllerH)
    $textH = $textH.Replace("~mode();", "~mode() = default;")
    $textH = $textH.Replace("~zone();", "~zone() = default;")
    [IO.File]::WriteAllText($controllerH, $textH, [Text.UTF8Encoding]::new($false))
}
$controllerCpp = Join-Path $SourceDir 'RGBController\RGBController.cpp'
if (Test-Path -LiteralPath $controllerCpp) {
    $textCpp = [IO.File]::ReadAllText($controllerCpp)
    $textCpp = [regex]::Replace($textCpp, '(?s)mode::~mode\(\)\s*\{.*?\}', '')
    $textCpp = [regex]::Replace($textCpp, '(?s)zone::~zone\(\)\s*\{.*?\}', '')
    [IO.File]::WriteAllText($controllerCpp, $textCpp, [Text.UTF8Encoding]::new($false))
}

Write-Host "Interface MasterRGB $Version aplicada (Inicio, Apoiar, icone, atualizacoes e Sobre)." -ForegroundColor Green
