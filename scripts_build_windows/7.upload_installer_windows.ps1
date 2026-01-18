# 7.upload_installer_windows.ps1
# Uploads Windows installer to GitHub Release for the current tag.

param(
    [string]$TokenEnvVar = "GITHUB_TOKEN",
    [string]$Tag = "",
    [string]$AssetPath = "",
    [string]$TargetCommitish = "beta",
    [switch]$SkipCreateRelease
)

function Resolve-ProjectRoot {
    param([string]$startDir)
    foreach ($dir in @($startDir, (Join-Path $startDir ".."), (Get-Location).Path)) {
        if ($dir -and (Test-Path (Join-Path $dir "CMakeLists.txt"))) {
            return (Resolve-Path $dir).Path
        }
    }
    throw "Nie znaleziono CMakeLists.txt - uruchom skrypt z katalogu projektu."
}

$ErrorActionPreference = "Stop"
$PROJECT_ROOT = Resolve-ProjectRoot -startDir $PSScriptRoot

if (-not $Tag) {
    $version = (Get-Content (Join-Path $PROJECT_ROOT "version.txt") -Raw).Trim()
    if ($version -match "^v") {
        $Tag = $version
    } else {
        $Tag = "v$version"
    }
}

if (-not $AssetPath) {
    $AssetPath = Join-Path $PROJECT_ROOT "release_exe\InwentaryzacjaInstaller.exe"
}

if (-not (Test-Path $AssetPath)) {
    throw "Nie znaleziono pliku instalatora: $AssetPath"
}

$token = [Environment]::GetEnvironmentVariable($TokenEnvVar)
if (-not $token) {
    throw "Brak tokena w zmiennej srodowiskowej $TokenEnvVar"
}

$remoteUrl = (git config --get remote.origin.url)
if (-not $remoteUrl) {
    throw "Nie znaleziono remote origin w git."
}

if ($remoteUrl -match "github.com[:/](.+?)/(.+?)(\.git)?$") {
    $owner = $Matches[1]
    $repo = $Matches[2]
} else {
    throw "Nie umiem odczytac owner/repo z URL: $remoteUrl"
}

$apiBase = "https://api.github.com/repos/$owner/$repo"
$headers = @{
    Authorization = "token $token"
    "User-Agent"  = "inwentaryzacja-upload"
    Accept        = "application/vnd.github+json"
}

Write-Host ">> Repo: $owner/$repo"
Write-Host ">> Tag: $Tag"
Write-Host ">> Asset: $AssetPath"

$release = $null
try {
    $release = Invoke-RestMethod -Method Get -Uri "$apiBase/releases/tags/$Tag" -Headers $headers
    Write-Host ">> Release istnieje, id=$($release.id)"
} catch {
    if ($SkipCreateRelease) {
        throw "Release dla tagu $Tag nie istnieje, a SkipCreateRelease jest ustawione."
    }

    $isPre = $Tag -like "*beta*" -or $Tag -like "*rc*"
    $body = @{
        tag_name         = $Tag
        name             = $Tag
        target_commitish = $TargetCommitish
        draft            = $false
        prerelease       = $isPre
    }
    Write-Host ">> Tworze release dla tagu $Tag (prerelease=$isPre)"
    $release = Invoke-RestMethod -Method Post -Uri "$apiBase/releases" -Headers $headers -Body ($body | ConvertTo-Json -Depth 5)
}

if (-not $release.upload_url) {
    throw "Brak upload_url w odpowiedzi z GitHub."
}

$assetName = [IO.Path]::GetFileName($AssetPath)
if ($release.assets) {
    $existing = $release.assets | Where-Object { $_.name -eq $assetName } | Select-Object -First 1
    if ($existing) {
        Write-Host ">> Usuwam istniejacy asset: $assetName"
        Invoke-RestMethod -Method Delete -Uri "$apiBase/releases/assets/$($existing.id)" -Headers $headers | Out-Null
    }
}

$uploadUrlRaw = $release.upload_url
$uploadUrl = $uploadUrlRaw -replace "\{.*\}", ""
$uploadUrl = $uploadUrl.Trim()
if ($uploadUrl -notmatch "^https?://") {
    throw "Nieprawidlowy upload_url: $uploadUrlRaw"
}
Write-Host ">> Upload..."
Write-Host ">> upload_url(raw): $uploadUrlRaw"
Write-Host ">> upload_url: $uploadUrl"
Invoke-RestMethod -Method Post `
    -Uri "$uploadUrl?name=$assetName" `
    -Headers $headers `
    -ContentType "application/octet-stream" `
    -InFile $AssetPath | Out-Null

Write-Host ">> Gotowe: $assetName"
