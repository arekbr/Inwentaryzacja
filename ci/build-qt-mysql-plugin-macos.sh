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

HOST_ARCH="$(uname -m)"
case "$HOST_ARCH" in
    arm64|x86_64) TARGET_ARCH="$HOST_ARCH" ;;
    *) echo "❌ Nieznana architektura runner'a: $HOST_ARCH"; exit 1 ;;
esac

echo "▶ Qt install prefix: $QT_PATH"
echo "▶ Qt version: $QT_VERSION"
echo "▶ Target arch: $TARGET_ARCH"

# 1. Install MariaDB (headers + libmariadb.dylib)
if ! brew list mariadb >/dev/null 2>&1; then
    echo "▶ Instaluję mariadb via brew…"
    brew install mariadb
else
    echo "✓ mariadb już zainstalowane"
fi

BREW_PREFIX="$(brew --prefix)"

MARIADB_LIB=""
for candidate in \
    "$BREW_PREFIX/opt/mariadb/lib/mariadb" \
    "$BREW_PREFIX/opt/mariadb/lib" \
    "$BREW_PREFIX/lib/mariadb" \
    "$BREW_PREFIX/lib"; do
    if [[ -f "$candidate/libmariadb.dylib" ]]; then
        MARIADB_LIB="$candidate"
        break
    fi
done
if [[ -z "$MARIADB_LIB" ]]; then
    echo "❌ Nie znaleziono libmariadb.dylib w żadnej z:"
    echo "   $BREW_PREFIX/{opt/mariadb/,}lib{/mariadb,}/libmariadb.dylib"
    find "$BREW_PREFIX" -name "libmariadb.dylib" -type f 2>/dev/null | head -5
    exit 1
fi

MARIADB_INCLUDE=""
for candidate in \
    "$BREW_PREFIX/opt/mariadb/include/mariadb" \
    "$BREW_PREFIX/opt/mariadb/include/mysql" \
    "$BREW_PREFIX/include/mariadb" \
    "$BREW_PREFIX/include/mysql"; do
    if [[ -f "$candidate/mysql.h" ]]; then
        MARIADB_INCLUDE="$candidate"
        break
    fi
done
if [[ -z "$MARIADB_INCLUDE" ]]; then
    echo "❌ Nie znaleziono nagłówka mysql.h w żadnej z:"
    echo "   $BREW_PREFIX/{opt/mariadb/,}include/{mariadb,mysql}/mysql.h"
    find "$BREW_PREFIX" -name "mysql.h" -type f 2>/dev/null | head -5
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
echo "▶ Qt QtCore framework arch:"
lipo -info "$QT_PATH/lib/QtCore.framework/Versions/A/QtCore" 2>&1 || true
echo "▶ MariaDB dylib arch:"
lipo -info "$MARIADB_LIB/libmariadb.dylib" 2>&1 || true

cat > "$BUILD_DIR/CMakeLists.txt" <<EOF
cmake_minimum_required(VERSION 3.20)
set(CMAKE_OSX_ARCHITECTURES "$TARGET_ARCH" CACHE STRING "" FORCE)
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
export CMAKE_OSX_ARCHITECTURES="$TARGET_ARCH"
cmake -G Ninja . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="$TARGET_ARCH" \
    -DCMAKE_HOST_SYSTEM_PROCESSOR="$TARGET_ARCH" \
    -DCMAKE_SYSTEM_PROCESSOR="$TARGET_ARCH" \
    -DMySQL_INCLUDE_DIRS="$MARIADB_INCLUDE" \
    -DMySQL_LIBRARIES="$MARIADB_LIB/libmariadb.dylib"

echo "▶ Effective OSX_ARCHITECTURES after configure:"
grep -E "^CMAKE_OSX_ARCHITECTURES" "$BUILD_DIR/CMakeCache.txt" || true

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
