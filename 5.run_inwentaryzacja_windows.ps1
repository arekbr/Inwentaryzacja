# 5.test_run_windows.ps1
$ErrorActionPreference = "Stop"

$APP_NAME = "Inwentaryzacja.exe"
$DEPLOY_DIR = "deploy"
$APP_PATH = Join-Path $DEPLOY_DIR $APP_NAME

Write-Host "`n🚀 Uruchamianie aplikacji $APP_NAME z katalogu $DEPLOY_DIR..." -ForegroundColor Cyan

if (!(Test-Path $APP_PATH)) {
    Write-Error "❌ Nie znaleziono $APP_NAME w $DEPLOY_DIR"
    exit 1
}

# ============================
# Ustawienie zmiennych środowiskowych
# ============================
$env:QT_QPA_PLATFORM_PLUGIN_PATH = ".\platforms"
$env:QT_PLUGIN_PATH = ".\sqldrivers"
$env:PATH = "$PWD;$env:PATH"

# ============================
# Uruchomienie aplikacji
# ============================
Push-Location $DEPLOY_DIR
try {
    Start-Process -FilePath ".\$APP_NAME" -Wait
} finally {
    Pop-Location
}
