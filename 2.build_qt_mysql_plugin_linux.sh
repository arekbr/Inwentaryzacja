#!/bin/bash

set -e

echo -e "\n🔌 [PLUGIN] Budowa Qt SQL Driverów (w tym qsqlmysql) — generator: Ninja\n"

# ==========================
# Sprawdzenie wymaganych zmiennych
# ==========================
if [[ -z "$QT_PATH" ]]; then
  echo "❌ QT_PATH nie jest ustawiony. Uruchom najpierw 'source ./bootstrap_linux.sh'."
  exit 1
fi

if [[ -z "$QT_SRC_PATH" ]]; then
  echo "❌ QT_SRC_PATH nie jest ustawiony. Brakuje źródeł Qt."
  exit 1
fi

SQLDRIVERS_SRC="$QT_SRC_PATH/qtbase/src/plugins/sqldrivers"
if [[ ! -d "$SQLDRIVERS_SRC" ]]; then
  echo "❌ Nie znaleziono katalogu sqldrivers w: $SQLDRIVERS_SRC"
  exit 1
fi

BUILD_DIR="build_qt_sql_drivers"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# ==========================
# Konfiguracja CMake z Ninja
# ==========================
cmake -G Ninja "$SQLDRIVERS_SRC" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$QT_PATH" \
  -DQT_FEATURE_sql_mysql=ON

# ==========================
# Budowanie
# ==========================
ninja -j$(nproc)

# ==========================
# Wyszukiwanie pluginu
# ==========================
PLUGIN=$(find . -name "libqsqlmysql.so" | head -n 1)
if [[ -f "$PLUGIN" ]]; then
  echo -e "\n✅ Zbudowano plugin: $PLUGIN"
#  echo "📥 Możesz go ręcznie skopiować do katalogu:"
#  echo "    $QT_PATH/plugins/sqldrivers/"
else
  echo "❌ Nie znaleziono libqsqlmysql.so. Coś poszło nie tak."
  exit 1
fi
