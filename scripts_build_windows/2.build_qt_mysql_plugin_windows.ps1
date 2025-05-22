# 2.build_qt_mysql_plugin_windows.ps1
Write-Host "`n🔌 [PLUGIN] Budowa Qt SQL Driverów (qsqlmysql) — generator: Ninja" -ForegroundColor Cyan
$ErrorActionPreference = "Stop"

$toolsDir = "tools"
$srcSubdir = "mariadb_src"
$buildSubdir = "mariadb_build"
$installSubdir = "mariadb_built"

# ========= [1] Wczytaj QT_PATH / QT_SRC_PATH =========
if (-not $env:QT_PATH -and (Test-Path "qt_env.ps1")) {
    . .\qt_env.ps1
}
$qtSrcMysqlDir = Join-Path $env:QT_SRC_PATH "qtbase/src/plugins/sqldrivers/mysql"
$cmakeListFile = Join-Path $qtSrcMysqlDir "CMakeLists.txt"
$cmakeListBackup = "$cmakeListFile.bak"
if (-not (Test-Path $cmakeListFile)) {
    Write-Error "❌ Nie znaleziono $cmakeListFile"
    exit 1
}

# ========= [2] Znajdź kompilator C++ w Qt =========
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

# ========= [3] Utwórz katalogi tools/* jeśli brak =========
foreach ($dir in @($toolsDir, "$toolsDir\$srcSubdir", "$toolsDir\$buildSubdir", "$toolsDir\$installSubdir")) {
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir | Out-Null
    }
}

# ========= [4] Pobierz i rozpakuj źródła MariaDB =========
$srcUrl = "https://downloads.mariadb.com/Connectors/c/connector-c-3.3.4/mariadb-connector-c-3.3.4-src.tar.gz"
$srcArchive = "$toolsDir/mariadb-src.tar.gz"
if (-not (Test-Path $srcArchive)) {
    Invoke-WebRequest -Uri $srcUrl -OutFile $srcArchive
}
tar -xf $srcArchive -C $toolsDir

# Zamień unpackowany folder na tools/mariadb_src
$unpacked = Get-ChildItem $toolsDir | Where-Object { $_.PSIsContainer -and $_.Name -like "mariadb-connector-c-*" } | Select-Object -First 1
Remove-Item "$toolsDir\$srcSubdir" -Recurse -Force -ErrorAction SilentlyContinue
Move-Item $unpacked.FullName "$toolsDir\$srcSubdir"

# ========= [5] Patch strerror_r → strerror_s =========
$maNet = Join-Path "$toolsDir\$srcSubdir\libmariadb" "ma_net.c"
if (Test-Path $maNet) {
    Write-Host "🩹 Patchuję $maNet → strerror_s(...)"
    (Get-Content $maNet) -replace 'strerror_r\((.*?),\s*(.*?),\s*(.*?)\);', 'strerror_s($2, $3, $1);' | Set-Content $maNet -Encoding UTF8
}

# ========= [6] Buduj MariaDB Connector/C =========
$buildDir = "$toolsDir\$buildSubdir"
$installDir = Resolve-Path "$toolsDir\$installSubdir"
Push-Location $buildDir

cmake ../$srcSubdir -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_INSTALL_PREFIX="$installDir" `
  -DWITH_SSL=OFF `
  -DWITH_EXTERNAL_ZLIB=NO `
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON `
  -DCMAKE_CXX_COMPILER="$CXX_COMPILER"

ninja install
Pop-Location

# ========= [7] Wylicz INCLUDE/LIB dla Qt pluginu =========
$INCLUDE_DIR = (Resolve-Path "$toolsDir\$installSubdir/include/mariadb").Path -replace '\\','/'
$LIB_DIR     = (Resolve-Path "$toolsDir\$installSubdir/lib/mariadb").Path -replace '\\','/'

# ========= [8] Patch Qt CMakeLists.txt (jeśli nie istnieje) =========
Write-Host "✏️ Tymczasowo patchuję CMakeLists.txt (dodaję alias MySQL::MySQL)..."
Copy-Item $cmakeListFile $cmakeListBackup -Force
$patch = @"
add_library(MySQL::MySQL UNKNOWN IMPORTED)
set_target_properties(MySQL::MySQL PROPERTIES
  IMPORTED_LOCATION "${LIB_DIR}/libmariadb.dll.a"
  INTERFACE_INCLUDE_DIRECTORIES "${INCLUDE_DIR}"
)

"@
if (-not (Select-String -Path $cmakeListFile -Pattern "MySQL::MySQL")) {
    Add-Content $cmakeListFile $patch
} else {
    Write-Host "ℹ️  Alias MySQL::MySQL już istnieje — patch pominięty"
}

# ========= [9] Przygotuj build_qt_sql_drivers od zera =========
$pluginBuildDir = "build_qt_sql_drivers"
if (Test-Path $pluginBuildDir) {
    Write-Host "🧽 Usuwam poprzednią konfigurację: $pluginBuildDir"
    Remove-Item -Recurse -Force $pluginBuildDir
}
New-Item -ItemType Directory -Path $pluginBuildDir | Out-Null
Set-Location $pluginBuildDir

# ========= [10] Konfiguracja i budowa Qt pluginu =========
Write-Host "`n⚙️  Konfiguruję CMake..."

cmake "$env:QT_SRC_PATH\qtbase\src\plugins\sqldrivers" -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH=$env:QT_PATH `
  -DQT_FEATURE_sql_mysql=ON `
  -DCMAKE_INCLUDE_PATH="$INCLUDE_DIR" `
  -DCMAKE_LIBRARY_PATH="$LIB_DIR" `
  -DCMAKE_CXX_COMPILER="$CXX_COMPILER"

Write-Host "`n🛠️  Buduję pluginy..."
ninja

# ========= [11] Przywrócenie CMakeLists.txt Qt =========
Write-Host "`n♻️ Przywracam oryginalny CMakeLists.txt..."
Move-Item -Path $cmakeListBackup -Destination $cmakeListFile -Force

# ========= [12] Sprawdzenie wyników =========
$plugin = Get-ChildItem -Recurse -Filter qsqlmysql.dll | Select-Object -First 1
if ($plugin) {
    Write-Host "`n✅ Zbudowano plugin: $($plugin.FullName)" -ForegroundColor Green
#    Write-Host "📥 Możesz go teraz skopiować do:"
#    Write-Host "    $env:QT_PATH\plugins\sqldrivers\"
} else {
    Write-Error "❌ Nie znaleziono qsqlmysql.dll. Coś poszło nie tak."
    exit 1
}

# ========= [13] Powrót do głównego katalogu =========
Set-Location $PSScriptRoot
