# 1.bootstrap_windows.ps1
# Przygotowanie środowiska Qt 6.9.0 + MariaDB (Windows)

Write-Host "`n📦 [BOOTSTRAP] Inicjalizacja środowiska pod Qt 6.9.0 + MariaDB (Windows)" -ForegroundColor Cyan

# ==========================================
# Krok 1: Sprawdzenie wersji systemu
# ==========================================
$version = (Get-CimInstance Win32_OperatingSystem).Version
if ($version -lt "10.0") {
    Write-Error "❌ Ten skrypt wymaga Windows 10 lub nowszego."
    exit 1
}
Write-Host "✅ Wykryto Windows 10/11"

# ==========================================
# Krok 2: Funkcja do wykrywania narzędzi
# ==========================================
function Ensure-Tool {
    param(
        [string]$name,
        [string]$fallbackSearchPath = ""
    )

    $tool = Get-Command $name -ErrorAction SilentlyContinue
    if ($tool) {
        Write-Host "✅ $name znaleziony w PATH: $($tool.Source)"
        try {
            $verRaw = & $tool.Source --version 2>&1 | Out-String
            if ($verRaw -match "\d+(\.\d+)+") {
                Write-Host "   ↪️  Wersja: $($Matches[0])"
            }
        } catch {
            Write-Host "   ⚠️  Nie udało się odczytać wersji $name"
        }
        return $true
    }

    if ($fallbackSearchPath -and (Test-Path $fallbackSearchPath)) {
        Write-Host "🔎 $name nie znaleziony w PATH – przeszukuję $fallbackSearchPath..."

        $found = Get-ChildItem -Recurse -Filter "$name.exe" -Path $fallbackSearchPath -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($found) {
            $dir = Split-Path $found.FullName
            $env:PATH = "$dir;$env:PATH"
            Write-Host "✅ $name znaleziony w: $dir (dodano do PATH)"

            try {
                $verRaw = & "$found.FullName" --version 2>&1 | Out-String
                if ($verRaw -match "\d+(\.\d+)+") {
                    Write-Host "   ↪️  Wersja: $($Matches[0])"
                }
            } catch {
                Write-Host "   ⚠️  Nie udało się odczytać wersji $name"
            }

            return $true
        }
    }

    Write-Error "❌ Nie znaleziono $name. Zainstaluj lub dodaj do PATH."
    return $false
}


# ==========================================
# Krok 3: Sprawdzenie wymaganych narzędzi
# ==========================================
$toolsOk = $true
$toolsOk = $toolsOk -and (Ensure-Tool "ninja" "C:\Qt\Tools")
$toolsOk = $toolsOk -and (Ensure-Tool "cmake" "C:\Qt\Tools")
$toolsOk = $toolsOk -and (Ensure-Tool "git" "C:\Program Files\Git\cmd")

if (-not $toolsOk) {
    exit 1
}

# ==========================================
# Krok 4: Ścieżka do Qt
# ==========================================
$defaultQtPath = "C:\Qt\6.9.0\mingw_64"
$detectedQt = Test-Path $defaultQtPath

$useQt = Read-Host "`n📂 Czy chcesz użyć własnej instalacji Qt 6.9.0? (y/n)"
$QT_PATH = ""
$QT_SRC_PATH = ""

if ($useQt -eq "y") {
    if ($detectedQt) {
        $QT_PATH = Read-Host "🔍 Podaj ścieżkę do katalogu Qt [domyślnie: $defaultQtPath]"
        if ([string]::IsNullOrWhiteSpace($QT_PATH)) {
            $QT_PATH = $defaultQtPath
        }
    } else {
        $QT_PATH = Read-Host "🔍 Podaj ścieżkę do katalogu Qt (np. C:\Qt\6.9.0\mingw_64)"
    }

    if (-not (Test-Path $QT_PATH)) {
        Write-Error "❌ Podana ścieżka nie istnieje."
        exit 1
    }

    Write-Host "✅ Qt PATH ustawiony na: $QT_PATH"

    $QT_SRC_PATH = Join-Path (Split-Path $QT_PATH -Parent) "Src"
    if (Test-Path "$QT_SRC_PATH\qtbase\src\plugins\sqldrivers\mysql") {
        Write-Host "✅ Wykryto źródła Qt: $QT_SRC_PATH"
    } else {
        Write-Warning "⚠️  Źródła Qt nie zostały odnalezione w: $QT_SRC_PATH"
        Write-Host "    Upewnij się, że Qt zostało zainstalowane z komponentem 'Src'"
    }

    # Ustawienie zmiennych środowiskowych
    $env:QT_PATH = $QT_PATH
    $env:QT_SRC_PATH = $QT_SRC_PATH
    $env:QMAKE = "$QT_PATH\bin\qmake.exe"
    $env:CMAKE_PREFIX_PATH = $QT_PATH
    $env:PATH = "$QT_PATH\bin;" + $env:PATH
} else {
    Write-Host "`n📥 Qt 6.9.0 zostanie pobrane i zbudowane lokalnie w kolejnym kroku (build_qt.ps1)"
}


# ==========================================
# Krok 5: Katalog build\
# ==========================================
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
    Write-Host "✅ Utworzono katalog build\"
} else {
    Write-Host "✅ Katalog build\ już istnieje"
}

# ==========================================
# Krok 6: Zapis qt_env.ps1
# ==========================================
@"
`$env:QT_PATH = '$QT_PATH'
`$env:QT_SRC_PATH = '$QT_SRC_PATH'
`$env:QMAKE = '$QT_PATH\bin\qmake.exe'
`$env:CMAKE_PREFIX_PATH = '$QT_PATH'
`$env:PATH = '$QT_PATH\bin;' + `$env:PATH
"@ | Set-Content -Path "qt_env.ps1" -Encoding UTF8

Write-Host "`n📄 Zapisano qt_env.ps1"

# ==========================================
# Krok 7: Zakończenie
# ==========================================
if ($QT_PATH -and $QT_SRC_PATH) {
    Write-Host "`n✅ Środowisko gotowe do budowy pluginu QMYSQL i projektu Inwentaryzacja"
    Write-Host "➡️  Kolejny krok: .\2.build_qt_mysql_plugin_windows.ps1"
} else {
    Write-Host "`n🛑 Qt nie jest jeszcze zainstalowane — kolejny krok to: .\build_qt.ps1 (pełna kompilacja Qt ze źródeł)"
}
