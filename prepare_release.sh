#!/bin/bash

set -e

APP_PATH="build/Qt_6_5_3_for_macOS-Release/Inwentaryzacja.app"
QT_PATH="/Users/Arek/Qt/6.5.3/macos"
DEPLOYQT="$QT_PATH/bin/macdeployqt"

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

