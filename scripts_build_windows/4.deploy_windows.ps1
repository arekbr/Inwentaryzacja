<# 4.deploy_windows.ps1
   Copyright (c) 2025 SMOK
   Deploy aplikacji Qt 6.x na Windows
#>

param (
    [string]$APP_NAME          = 'Inwentaryzacja.exe',
    [string]$BUILD_DIR         = 'build_inwentaryzacja',
    [string]$PLUGIN_BUILD_DIR  = 'build_qt_sql_drivers',  # katalog z własnym qsqlmysql.dll
    [string]$DEPLOY_DIR        = 'deploy'
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
function Resolve-ProjectPath {
    param([string]$basePath, [string]$projectRoot)
    if ([IO.Path]::IsPathRooted($basePath)) { return $basePath }
    return (Join-Path $projectRoot $basePath)
}
$PROJECT_ROOT = Resolve-ProjectRoot -startDir $PSScriptRoot
$BUILD_DIR = Resolve-ProjectPath -basePath $BUILD_DIR -projectRoot $PROJECT_ROOT
$PLUGIN_BUILD_DIR = Resolve-ProjectPath -basePath $PLUGIN_BUILD_DIR -projectRoot $PROJECT_ROOT
$DEPLOY_DIR = Resolve-ProjectPath -basePath $DEPLOY_DIR -projectRoot $PROJECT_ROOT

$ErrorActionPreference = 'Stop'
Write-Host "`n🚀  Deploy aplikacji Windows: $APP_NAME" -ForegroundColor Cyan

# ------------------------------------------------------------------
# 1. QT_PATH: pobierz z env lub z pliku qt_env.ps1
# ------------------------------------------------------------------
if (-not $env:QT_PATH -and (Test-Path (Join-Path $PROJECT_ROOT "qt_env.ps1"))) {
    Write-Host "??  Wczytywanie zmiennych z qt_env.ps1"
    . (Join-Path $PROJECT_ROOT "qt_env.ps1")
}
if (-not $env:QT_PATH) {
    throw '❌  Zmienna środowiskowa QT_PATH nie jest ustawiona (np. C:\Qt\6.6.2\msvc64).'
}

$QT_BIN     = Join-Path $env:QT_PATH 'bin'
$QT_PLUGINS = Join-Path $env:QT_PATH 'plugins'
$WINDEPLOY  = Join-Path $QT_BIN      'windeployqt.exe'

if (-not (Test-Path $WINDEPLOY)) {
    throw "❌  Nie znaleziono windeployqt w $QT_BIN"
}

# ------------------------------------------------------------------
# 2. Oczyść i utwórz katalog deploy/
# ------------------------------------------------------------------
if (Test-Path $DEPLOY_DIR) {
    Write-Host "🧹  Czyszczenie katalogu $DEPLOY_DIR..."
    Remove-Item -Recurse -Force $DEPLOY_DIR
}
New-Item -ItemType Directory -Path $DEPLOY_DIR | Out-Null

# ------------------------------------------------------------------
# 3. Skopiuj zbudowane .exe
# ------------------------------------------------------------------
$exeSrc = Join-Path $BUILD_DIR $APP_NAME
if (-not (Test-Path $exeSrc)) {
    throw "❌  Nie znaleziono $exeSrc – upewnij się, że aplikacja została zbudowana."
}
Copy-Item $exeSrc $DEPLOY_DIR
$exeDst = Join-Path $DEPLOY_DIR $APP_NAME

# ------------------------------------------------------------------
# 4. Uruchom windeployqt (dodaje wszystkie plug-iny i zależności Qt)
# ------------------------------------------------------------------
Write-Host "🚚  Uruchamiam windeployqt..."
& "$WINDEPLOY" `
    "$exeDst" `
    --release `
    --no-translations `
    --compiler-runtime `
    --verbose 1 `
    | Write-Host

# ------------------------------------------------------------------
# 5. Dołóż ręcznie zbudowany qsqlmysql.dll
# ------------------------------------------------------------------
$mysqlDstDir = Join-Path $DEPLOY_DIR 'plugins\sqldrivers'
New-Item -ItemType Directory -Force -Path $mysqlDstDir | Out-Null

$qsqlmysql = Join-Path $PLUGIN_BUILD_DIR 'plugins\sqldrivers\qsqlmysql.dll'
if (Test-Path $qsqlmysql) {
    Copy-Item $qsqlmysql $mysqlDstDir -Force
    Write-Host "✅  Dodano własny qsqlmysql.dll"
} else {
    Write-Warning "⚠️  Nie znaleziono $qsqlmysql – driver MySQL nie został skopiowany."
}

# ------------------------------------------------------------------
# 6. Dołóż libmariadb.dll (jeśli istnieje)
# ------------------------------------------------------------------
$libmariadb = Get-ChildItem -Recurse -Filter libmariadb*.dll -Path $PROJECT_ROOT -ErrorAction SilentlyContinue | Select-Object -First 1
if ($libmariadb) {
    Copy-Item $libmariadb.FullName $DEPLOY_DIR -Force
    Write-Host "✅  Dodano $($libmariadb.Name)"
} else {
    Write-Warning "⚠️  libmariadb.dll nie znalezione – jeśli aplikacja używa MySQL/MariaDB bez TLS, wszystko nadal zadziała."
}

# ------------------------------------------------------------------
# 7. Raport końcowy
# ------------------------------------------------------------------
Write-Host "`n🎉  Gotowe!  Zawartość paczki w '$DEPLOY_DIR\'" -ForegroundColor Green
Write-Host "   cd $DEPLOY_DIR"
Write-Host "   .\\$APP_NAME"
