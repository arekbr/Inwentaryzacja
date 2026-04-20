#!/bin/bash
# Build Qt QMYSQL SQL driver plugin on macOS CI runners.
#
# Qt distribuowane przez aqtinstall nie zawiera pluginu MySQL/MariaDB
# (kwestia licencji). Żeby installer macOS miał driver QMYSQL, trzeba go
# zbudować z Qt source'ów qtbase dla tej samej wersji Qt, a następnie
# skopiować do $QT_PATH/plugins/sqldrivers — wtedy package-macos.sh
# i macdeployqt podchwyci go automatycznie.
#
# Wymagania:
#   - Qt zainstalowane (qmake w PATH)
#   - Python z aqtinstall (aqt)
#   - brew (zainstaluje mariadb)
#   - ninja + cmake (standardowo na runnerach macOS GitHub Actions)
set -euo pipefail

QT_VERSION="${QT_VERSION:-6.8.2}"
QT_PATH="$(qmake -query QT_INSTALL_PREFIX)"
SRC_OUT="${SRC_OUT:-$RUNNER_TEMP/QtSrc}"
BUILD_DIR="${BUILD_DIR:-$RUNNER_TEMP/build_qmysql}"

echo "▶ Qt install prefix: $QT_PATH"
echo "▶ Qt version: $QT_VERSION"

# 1. Install MariaDB (headers + libmariadb.dylib)
if ! brew list mariadb >/dev/null 2>&1; then
    echo "▶ Instaluję mariadb via brew…"
    brew install mariadb
else
    echo "✓ mariadb już zainstalowane"
fi

BREW_PREFIX="$(brew --prefix)"
MARIADB_LIB="$BREW_PREFIX/lib"
MARIADB_INCLUDE="$BREW_PREFIX/include/mariadb"

if [[ ! -f "$MARIADB_LIB/libmariadb.dylib" ]]; then
    echo "❌ Brak $MARIADB_LIB/libmariadb.dylib — sprawdź instalację brew"
    exit 1
fi
if [[ ! -f "$MARIADB_INCLUDE/mysql.h" ]]; then
    echo "❌ Brak nagłówka $MARIADB_INCLUDE/mysql.h"
    exit 1
fi
echo "✓ MariaDB lib: $MARIADB_LIB/libmariadb.dylib"
echo "✓ MariaDB include: $MARIADB_INCLUDE"

# 2. Pobierz Qt source dla qtbase
if [[ ! -d "$SRC_OUT/$QT_VERSION/Src/qtbase/src/plugins/sqldrivers" ]]; then
    echo "▶ Pobieram Qt sources (qtbase) przez aqt…"
    python -m aqt install-src mac "$QT_VERSION" --archives qtbase --outputdir "$SRC_OUT"
fi

SQLDRIVERS_SRC="$SRC_OUT/$QT_VERSION/Src/qtbase/src/plugins/sqldrivers"
if [[ ! -d "$SQLDRIVERS_SRC/mysql" ]]; then
    echo "❌ Brakuje źródeł mysql drivera w: $SQLDRIVERS_SRC/mysql"
    ls -la "$SQLDRIVERS_SRC/" || true
    exit 1
fi
echo "✓ SQL drivers sources: $SQLDRIVERS_SRC"

# 3. Wygeneruj CMakeLists dla pluginu i buduj
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cat > "$BUILD_DIR/CMakeLists.txt" <<EOF
cmake_minimum_required(VERSION 3.20)
project(BuildQMYSQL LANGUAGES CXX)
set(CMAKE_PREFIX_PATH "$QT_PATH")
set(QT_FEATURE_sql_mysql ON)

add_library(MySQL::MySQL UNKNOWN IMPORTED)
set_target_properties(MySQL::MySQL PROPERTIES
    IMPORTED_LOCATION "$MARIADB_LIB/libmariadb.dylib"
    INTERFACE_INCLUDE_DIRECTORIES "$MARIADB_INCLUDE"
)

add_subdirectory("$SQLDRIVERS_SRC" sqldrivers)
EOF

echo "▶ cmake configure…"
cd "$BUILD_DIR"
cmake -G Ninja . \
    -DCMAKE_BUILD_TYPE=Release \
    -DMySQL_INCLUDE_DIRS="$MARIADB_INCLUDE" \
    -DMySQL_LIBRARIES="$MARIADB_LIB/libmariadb.dylib"

echo "▶ cmake build…"
cmake --build . -j "$(sysctl -n hw.logicalcpu 2>/dev/null || echo 4)"

# 4. Znajdź plugin i skopiuj do Qt plugins
PLUGIN="$(find "$BUILD_DIR" -name "libqsqlmysql.dylib" -type f | head -n1)"
if [[ -z "$PLUGIN" ]]; then
    echo "❌ Nie znaleziono libqsqlmysql.dylib po buildzie"
    echo "🔍 Dostępne pliki .dylib:"
    find "$BUILD_DIR" -name "*.dylib" -type f
    exit 1
fi

echo "✓ Plugin: $PLUGIN"
file "$PLUGIN"

mkdir -p "$QT_PATH/plugins/sqldrivers"
cp "$PLUGIN" "$QT_PATH/plugins/sqldrivers/libqsqlmysql.dylib"
echo "✅ Skopiowano do $QT_PATH/plugins/sqldrivers/libqsqlmysql.dylib"

# 5. Weryfikacja — zależności pluginu
echo "▶ Zależności pluginu:"
otool -L "$QT_PATH/plugins/sqldrivers/libqsqlmysql.dylib"
