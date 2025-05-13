#!/bin/bash
set -e

echo -e "\n🔌 [PLUGIN] Budowa Qt SQL Driverów (w tym qsqlmysql) — generator: Ninja (macOS)\n"

# ==========================
# Wczytanie zmiennych środowiskowych
# ==========================
if [[ -z "$QT_PATH" && -f qt_env.sh ]]; then
  echo "ℹ️  Wczytywanie QT_PATH z qt_env.sh"
  source qt_env.sh
fi

if [[ -z "$QT_PATH" || -z "$QT_SRC_PATH" ]]; then
  echo "❌ QT_PATH lub QT_SRC_PATH nie jest ustawiony. Uruchom najpierw './bootstrap_macos.sh'."
  exit 1
fi

# ==========================
# Sprawdzenie źródeł pluginu
# ==========================
SQLDRIVERS_SRC="$QT_SRC_PATH/qtbase/src/plugins/sqldrivers"
if [[ ! -d "$SQLDRIVERS_SRC/mysql" ]]; then
  echo "❌ Brakuje źródeł mysql drivera w: $SQLDRIVERS_SRC/mysql"
  exit 1
fi

# ==========================
# Ścieżka do MariaDB (Homebrew)
# ==========================
MARIADB_PREFIX=$(brew --prefix mariadb)
if [[ ! -f "$MARIADB_PREFIX/lib/libmariadb.dylib" ]]; then
  echo "❌ Nie znaleziono libmariadb.dylib w $MARIADB_PREFIX"
  exit 1
fi

echo "✅ MariaDB wykryto w: $MARIADB_PREFIX"

# ==========================
# Katalog build
# ==========================
BUILD_DIR="build_qt_sql_drivers"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# ==========================
# Tworzenie wrappera CMakeLists.txt
# ==========================
echo "📝 Tworzenie tymczasowego wrappera CMakeLists.txt"

cat > CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.20)
project(BuildQMYSQL LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "$QT_PATH")
set(QT_FEATURE_sql_mysql ON)

# Utwórz alias MySQL::MySQL dla Qt 6.5+
add_library(MySQL::MySQL UNKNOWN IMPORTED)
set_target_properties(MySQL::MySQL PROPERTIES
    IMPORTED_LOCATION "$MARIADB_PREFIX/lib/libmariadb.dylib"
    INTERFACE_INCLUDE_DIRECTORIES "$MARIADB_PREFIX/include/mysql"
)

add_subdirectory("$SQLDRIVERS_SRC" sqldrivers)
EOF

# ==========================
# Budowa z CMake + Ninja
# ==========================
echo "⚙️  Konfiguracja CMake + budowa Ninja"
cmake -G Ninja . \
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_SYSTEM_PROCESSOR=$(uname -m)

ninja -j$(sysctl -n hw.logicalcpu)

# ==========================
# Weryfikacja
# ==========================
PLUGIN=$(find . -name "libqsqlmysql.dylib" | head -n1)
if [[ -f "$PLUGIN" ]]; then
  echo -e "\n✅ Zbudowano plugin: $PLUGIN"
else
  echo "❌ Nie znaleziono libqsqlmysql.dylib. Coś poszło nie tak."
  exit 1
fi
