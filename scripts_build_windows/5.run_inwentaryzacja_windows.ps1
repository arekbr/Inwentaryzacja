# 5.test_run_windows.ps1
# Project-root aware paths
function Resolve-ProjectRoot {
    param([string]$startDir)
    foreach ($dir in @($startDir, (Join-Path $startDir ".."), (Get-Location).Path)) {
        if ($dir -and (Test-Path (Join-Path $dir "CMakeLists.txt"))) {
            return (Resolve-Path $dir).Path
        }
    }
    throw "Nie znaleziono CMakeLists.txt - uruchom skrypt z katalogu projektu."
}
function Resolve-ProjectPath {
    param([string]$basePath, [string]$projectRoot)
    if ([IO.Path]::IsPathRooted($basePath)) { return $basePath }
    return (Join-Path $projectRoot $basePath)
}
$PROJECT_ROOT = Resolve-ProjectRoot -startDir $PSScriptRoot

$ErrorActionPreference = "Stop"

$APP_NAME = "Inwentaryzacja.exe"
$DEPLOY_DIR = Resolve-ProjectPath -basePath "deploy" -projectRoot $PROJECT_ROOT
$APP_PATH = Join-Path $DEPLOY_DIR $APP_NAME

Write-Host "`n🚀 Uruchamianie aplikacji $APP_NAME z katalogu $DEPLOY_DIR..." -ForegroundColor Cyan

if (!(Test-Path $APP_PATH)) {
    Write-Error "❌ Nie znaleziono $APP_NAME w $DEPLOY_DIR"
    exit 1
}

# ============================
# Ustawienie zmiennych środowiskowych
# ============================
$env:QT_QPA_PLATFORM_PLUGIN_PATH = Join-Path $DEPLOY_DIR "platforms"
$env:QT_PLUGIN_PATH = Join-Path $DEPLOY_DIR "plugins"
$env:PATH = "$DEPLOY_DIR;$env:PATH"

# ============================
# Uruchomienie aplikacji
# ============================
Push-Location $DEPLOY_DIR
try {
    Start-Process -FilePath ".\$APP_NAME" -Wait
} finally {
    Pop-Location
}
