#!/bin/bash

set -e

echo -e "\n🔌 [PLUGIN] Budowa Qt SQL Driverów (w tym qsqlmysql) — generator: Ninja\n"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# ==========================
# Sprawdzenie wymaganych zmiennych
# ==========================
if [[ -z "$QT_PATH" && -f "$ROOT_DIR/qt_env.sh" ]]; then
    echo "ℹ️  Wczytywanie QT_PATH z qt_env.sh"
    source "$ROOT_DIR/qt_env.sh"
fi

if [[ -z "$QT_PATH" ]]; then
  echo "❌ QT_PATH nie jest ustawiony. Uruchom 1.bootstrap_linux.sh lub ustaw ręcznie."
  exit 1
fi

QT_PLUGIN_DIR=""
if command -v qtpaths6 >/dev/null 2>&1; then
  QT_PLUGIN_DIR="$(qtpaths6 --plugin-dir)"
elif command -v qtpaths >/dev/null 2>&1; then
  QT_PLUGIN_DIR="$(qtpaths --plugin-dir)"
fi

if [[ -n "$QT_PLUGIN_DIR" && -f "$QT_PLUGIN_DIR/sqldrivers/libqsqlmysql.so" ]]; then
  echo "✅ Wykryto systemowy plugin qsqlmysql: $QT_PLUGIN_DIR/sqldrivers/libqsqlmysql.so"
  echo "➡️  Pomijam budowe pluginu (zainstalowany w systemie)."
  exit 0
fi

if [[ -z "$QT_SRC_PATH" ]]; then
  echo "❌ QT_SRC_PATH nie jest ustawiony. Brakuje źródeł Qt."
  echo "   Zainstaluj pakiet: libqt6sql6-mysql lub źródła Qt i ustaw QT_SRC_PATH."
  exit 1
fi

SQLDRIVERS_SRC="$QT_SRC_PATH/qtbase/src/plugins/sqldrivers"
if [[ ! -d "$SQLDRIVERS_SRC" ]]; then
  echo "❌ Nie znaleziono katalogu sqldrivers w: $SQLDRIVERS_SRC"
  exit 1
fi

BUILD_DIR="$ROOT_DIR/build_qt_sql_drivers"
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
