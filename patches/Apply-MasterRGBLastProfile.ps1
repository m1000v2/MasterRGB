[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$SourceDir
)

$ErrorActionPreference = 'Stop'
$profileManagerPath = Join-Path $SourceDir 'ProfileManager.cpp'
if (-not (Test-Path -LiteralPath $profileManagerPath)) {
    throw "ProfileManager.cpp não encontrado em '$SourceDir'."
}
$source = Get-Content -LiteralPath $profileManagerPath -Raw
$marker = 'MASTER RGB: remember last successfully loaded profile'
if ($source.Contains($marker)) {
    Write-Host 'Registro do último perfil já aplicado.' -ForegroundColor Yellow
    exit 0
}

# In this exact tested branch, both the profile dropdown and CLI call
# ProfileManager::LoadProfile(). Persist only after LoadProfileWithOptions
# succeeds, so the file always represents the last profile actually applied.
$pattern = '(?s)bool\s+ProfileManager::LoadProfile\s*\(\s*std::string\s+profile_name\s*\)\s*\{\s*profile_name\s*=\s*StringUtils::remove_null_terminating_chars\(profile_name\);\s*return\s*\(LoadProfileWithOptions\(profile_name,\s*false,\s*true\)\);\s*\}'
if (-not [regex]::IsMatch($source, $pattern)) {
    throw 'Não encontrei o método ProfileManager::LoadProfile esperado na branch release_candidate_1.0rc2wr0. Nenhuma alteração aplicada.'
}
$replacement = @'
bool ProfileManager::LoadProfile(std::string profile_name)
{
    profile_name = StringUtils::remove_null_terminating_chars(profile_name);
    bool loaded = LoadProfileWithOptions(profile_name, false, true);

    /* MASTER RGB: remember last successfully loaded profile */
    if(loaded)
    {
        std::string saved_profile = profile_name;
        const std::string profile_extension = ".orp";
        if(saved_profile.size() >= profile_extension.size()
         && saved_profile.compare(saved_profile.size() - profile_extension.size(), profile_extension.size(), profile_extension) == 0)
        {
            saved_profile.erase(saved_profile.size() - profile_extension.size());
        }
        std::ofstream last_profile_file(configuration_directory / filesystem::u8path("MasterRGB_LastProfile.txt"), std::ios::out | std::ios::trunc);
        if(last_profile_file.is_open())
        {
            last_profile_file << saved_profile;
        }
    }

    return loaded;
}
'@
$patched = [regex]::Replace($source, $pattern, [System.Text.RegularExpressions.MatchEvaluator]{ param($m) $replacement }, 1)
if ($patched -eq $source) { throw 'A alteração de persistência não foi aplicada.' }
[System.IO.File]::WriteAllText($profileManagerPath, $patched, [System.Text.UTF8Encoding]::new($false))
Write-Host 'Último perfil realmente carregado será registrado em %APPDATA%\\OpenRGB\\MasterRGB_LastProfile.txt.' -ForegroundColor Green
