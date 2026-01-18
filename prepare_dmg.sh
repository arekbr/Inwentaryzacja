#!/bin/bash

set -euo pipefail

### 🔧 KONFIGURACJA ###
APP_NAME="Inwentaryzacja"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR" && pwd)"

QT_PATH="${QT_PATH:-/Users/Arek/Qt/6.5.3/macos}"
DEPLOYQT="$QT_PATH/bin/macdeployqt"
APP_DIR="${APP_DIR:-$ROOT_DIR/build/Qt_6_5_3_for_macOS-Release/${APP_NAME}.app}"
DMG_NAME="${DMG_NAME:-${APP_NAME}_macOS.dmg}"

if [[ ! -d "$APP_DIR" ]]; then
    echo "❌ Brak aplikacji: $APP_DIR"
    exit 1
fi

if [[ ! -x "$DEPLOYQT" ]]; then
    echo "❌ Brak narzedzia macdeployqt: $DEPLOYQT"
    exit 1
fi

echo "🚀 [1/4] macdeployqt..."
"$DEPLOYQT" "$APP_DIR" -verbose=1

echo "🧹 [2/4] Czyszczenie nieużywanych pluginów SQL..."
rm -vf "$APP_DIR/Contents/PlugIns/sqldrivers/libqsqlodbc.dylib"
rm -vf "$APP_DIR/Contents/PlugIns/sqldrivers/libqsqlpsql.dylib"

echo "📄 [3/4] Sprawdzenie qt.conf..."
QT_CONF="$APP_DIR/Contents/Resources/qt.conf"
if [ ! -f "$QT_CONF" ]; then
    echo "[Paths]" > "$QT_CONF"
    echo "Plugins = PlugIns" >> "$QT_CONF"
else
    grep -q "\[Paths\]" "$QT_CONF" || echo "[Paths]" >> "$QT_CONF"
    grep -q "Plugins = PlugIns" "$QT_CONF" || echo "Plugins = PlugIns" >> "$QT_CONF"
fi

echo "💿 [4/4] Tworzenie prostego DMG..."
TMP_DMG="$(mktemp -d)"
cp -R "$APP_DIR" "$TMP_DMG/"

hdiutil create -volname "$APP_NAME" -srcfolder "$TMP_DMG" -ov -format UDZO "$DMG_NAME"
rm -rf "$TMP_DMG"

echo "✅ Gotowe! Wygenerowano: $DMG_NAME"
