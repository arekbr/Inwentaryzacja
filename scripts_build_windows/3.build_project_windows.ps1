# 3.build_project_windows.ps1
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
$PROJECT_ROOT = Resolve-ProjectRoot -startDir $PSScriptRoot

$ErrorActionPreference = "Stop"
$APP_NAME = "Inwentaryzacja"
$BUILD_DIR = Join-Path $PROJECT_ROOT "build_inwentaryzacja"

Write-Host "`n🏗️  Budowa aplikacji $APP_NAME (Windows + Qt)" -ForegroundColor Cyan

# ============================
# Znajdź kompilator C++ w Qt
# ============================
function Find-QtCompiler {
    $qtToolsPath = "C:\Qt\Tools"
    if (-not (Test-Path $qtToolsPath)) {
        Write-Error "❌ Nie znaleziono katalogu Qt Tools: $qtToolsPath"
        exit 1
    }

    $compiler = Get-ChildItem -Recurse -Path $qtToolsPath -Filter "c++.exe" | Select-Object -First 1
    if (-not $compiler) {
        Write-Error "❌ Nie znaleziono kompilatora C++ w katalogu Qt Tools"
        exit 1
    }

    Write-Host "✅ Znaleziono kompilator C++: $($compiler.FullName)"
    return $compiler.FullName
}

$CXX_COMPILER = Find-QtCompiler

# ============================
# Wczytywanie QT_PATH
# ============================
if (-not $env:QT_PATH -and (Test-Path (Join-Path $PROJECT_ROOT "qt_env.ps1"))) {
    Write-Host "??  Wczytywanie zmiennych z qt_env.ps1"
    . (Join-Path $PROJECT_ROOT "qt_env.ps1")
}

if (-not $env:QT_PATH) {
    Write-Error "❌ Zmienna QT_PATH nie jest ustawiona. Uruchom 1.bootstrap_windows.ps1 lub ustaw ręcznie."
    exit 1
}

# ============================
# Tworzenie katalogu build
# ============================
if (-not (Test-Path $BUILD_DIR)) {
    Write-Host "📁 Tworzenie katalogu: $BUILD_DIR"
    New-Item -ItemType Directory -Path $BUILD_DIR | Out-Null
}

# ============================
# Konfiguracja CMake
# ============================
Write-Host "⚙️  Konfiguracja CMake..."
cmake -G Ninja -S "$PROJECT_ROOT" -B "$BUILD_DIR" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$env:QT_PATH" `
  -DCMAKE_CXX_COMPILER="$CXX_COMPILER"

# ============================
# Budowanie
# ============================
Write-Host "🔨 Budowanie projektu z użyciem Ninja..."
cmake --build "$BUILD_DIR"

# ============================
# Weryfikacja binarki
# ============================
if (Test-Path (Join-Path $BUILD_DIR "$APP_NAME.exe")) {
    Write-Host "`n✅ Zbudowano aplikację: $BUILD_DIR\$APP_NAME.exe" -ForegroundColor Green
} else {
    Write-Error "❌ Budowa nie powiodła się — brak pliku $APP_NAME.exe"
    exit 1
}

# ========= [5] Powrót do głównego katalogu =========
