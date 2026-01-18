#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR" && pwd)"

APP_PATH="${APP_PATH:-$ROOT_DIR/build/Qt_6_5_3_for_macOS-Release/Inwentaryzacja.app}"
QT_PATH="${QT_PATH:-/Users/Arek/Qt/6.5.3/macos}"
DEPLOYQT="$QT_PATH/bin/macdeployqt"

if [[ ! -d "$APP_PATH" ]]; then
    echo "❌ Brak aplikacji: $APP_PATH"
    exit 1
fi

if [[ ! -x "$DEPLOYQT" ]]; then
    echo "❌ Brak narzedzia macdeployqt: $DEPLOYQT"
    exit 1
fi

echo "🚀 Uruchamianie macdeployqt..."
"$DEPLOYQT" "$APP_PATH" -verbose=1

echo "🧹 Czyszczenie nieużywanych driverów SQL (po deployqt)..."
rm -vf "$APP_PATH/Contents/PlugIns/sqldrivers/libqsqlodbc.dylib"
rm -vf "$APP_PATH/Contents/PlugIns/sqldrivers/libqsqlpsql.dylib"

echo "📄 Upewnianie się, że qt.conf zawiera ścieżkę Plugins..."
QT_CONF="$APP_PATH/Contents/Resources/qt.conf"

# Tworzymy qt.conf, jeśli nie istnieje
if [ ! -f "$QT_CONF" ]; then
    echo "[Paths]" > "$QT_CONF"
    echo "Plugins = PlugIns" >> "$QT_CONF"
else
    grep -q "\[Paths\]" "$QT_CONF" || echo "[Paths]" >> "$QT_CONF"
    grep -q "Plugins = PlugIns" "$QT_CONF" || echo "Plugins = PlugIns" >> "$QT_CONF"
fi

echo "✅ Gotowe! Aplikacja przygotowana do dystrybucji."
