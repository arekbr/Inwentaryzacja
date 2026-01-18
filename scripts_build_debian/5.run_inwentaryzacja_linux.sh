#!/bin/bash

set -e

APP_NAME="Inwentaryzacja"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
DEPLOY_DIR="$ROOT_DIR/deploy"

# ============================
# Wczytanie QT_PATH z pliku, jeśli nie jest ustawiony
# ============================
if [[ -z "$QT_PATH" && -f "$ROOT_DIR/qt_env.sh" ]]; then
    echo "ℹ️  Wczytywanie QT_PATH z qt_env.sh"
    source "$ROOT_DIR/qt_env.sh"
fi

if [[ -z "$QT_PATH" ]]; then
    echo "❌ QT_PATH nie jest ustawiony. Ustaw QT_PATH lub uruchom 1.bootstrap_linux.sh."
    exit 1
fi

# ============================
# Uruchamianie aplikacji z lokalnymi bibliotekami
# ============================
echo "🚀 Uruchamianie aplikacji $APP_NAME z katalogu $DEPLOY_DIR..."
cd "$DEPLOY_DIR"

QT_QPA_PLATFORM_PLUGIN_PATH=./platforms \
QT_PLUGIN_PATH=./sqldrivers \
LD_LIBRARY_PATH=.:$QT_PATH/lib \
./$APP_NAME
