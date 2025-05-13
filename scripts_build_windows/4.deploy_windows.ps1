# 4.deploy_windows.ps1
$ErrorActionPreference = "Stop"
$APP_NAME = "Inwentaryzacja.exe"
$BUILD_DIR = "build_inwentaryzacja"
$PLUGIN_BUILD_DIR = "build_qt_sql_drivers"
$DEPLOY_DIR = "deploy"

Write-Host "`n🚀 Deploy aplikacji Windows: $APP_NAME" -ForegroundColor Cyan

# ============================
# Wczytanie QT_PATH z pliku, jeśli nie jest ustawiony
# ============================
if (-not $env:QT_PATH -and (Test-Path "qt_env.ps1")) {
    Write-Host "ℹ️  Wczytywanie zmiennych z qt_env.ps1"
    . .\qt_env.ps1
}
if (-not $env:QT_PATH) {
    Write-Error "❌ Zmienna QT_PATH nie jest ustawiona."
    exit 1
}

$QT_BIN = Join-Path $env:QT_PATH "bin"
$QT_PLUGINS = Join-Path $env:QT_PATH "plugins"

# ============================
# Czyszczenie deploy/
# ============================
if (Test-Path $DEPLOY_DIR) {
    Write-Host "🧹 Czyszczenie katalogu $DEPLOY_DIR..."
    Remove-Item -Recurse -Force $DEPLOY_DIR
}
New-Item -ItemType Directory -Path $DEPLOY_DIR | Out-Null

# ============================
# Kopiowanie EXE
# ============================
Copy-Item "$BUILD_DIR\$APP_NAME" "$DEPLOY_DIR\"

# ============================
# Tworzenie podkatalogów pluginów
# ============================
New-Item -ItemType Directory -Force -Path "$DEPLOY_DIR\platforms" | Out-Null
New-Item -ItemType Directory -Force -Path "$DEPLOY_DIR\sqldrivers" | Out-Null

# ============================
# Kopiowanie pluginów Qt
# ============================
Copy-Item "$QT_PLUGINS\platforms\qwindows.dll" "$DEPLOY_DIR\platforms\" -ErrorAction SilentlyContinue
Copy-Item "$QT_PLUGINS\sqldrivers\qsqlite.dll" "$DEPLOY_DIR\sqldrivers\" -ErrorAction SilentlyContinue

# ============================
# Kopiowanie zbudowanego qsqlmysql.dll
# ============================
$qsqlmysql = "$PLUGIN_BUILD_DIR\plugins\sqldrivers\qsqlmysql.dll"
if (Test-Path $qsqlmysql) {
    Copy-Item $qsqlmysql "$DEPLOY_DIR\sqldrivers\"
    Write-Host "✅ Skopiowano qsqlmysql.dll"
} else {
    Write-Error "❌ Nie znaleziono qsqlmysql.dll. Czy plugin został zbudowany?"
    exit 1
}

# ============================
# Kopiowanie zależności DLL
# ============================
Write-Host "📦 Kopiowanie DLL z Qt bin/..."
$qt_dlls = @("Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll", "Qt6Sql.dll")
foreach ($dll in $qt_dlls) {
    $src = Join-Path $QT_BIN $dll
    if (Test-Path $src) {
        Copy-Item $src "$DEPLOY_DIR\"
    } else {
        Write-Warning "⚠️ Brak $dll"
    }
}

# ============================
# Kopiowanie libmariadb.dll jeśli istnieje
# ============================
$libmariadb = Get-ChildItem -Recurse -Filter libmariadb.dll -Path "tools" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($libmariadb) {
    Copy-Item $libmariadb.FullName "$DEPLOY_DIR\"
    Write-Host "✅ Skopiowano libmariadb.dll"
} else {
    Write-Warning "⚠️ Nie znaleziono libmariadb.dll — sprawdź czy connector został poprawnie zbudowany"
}

# ============================
# Gotowe
# ============================
Write-Host "`n✅ Deploy zakończony: zawartość w '$DEPLOY_DIR\'"
Write-Host "💡 Aby uruchomić aplikację:"
Write-Host "   cd $DEPLOY_DIR"
Write-Host "   .\$APP_NAME"
