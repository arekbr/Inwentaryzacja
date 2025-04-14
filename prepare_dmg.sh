#!/bin/bash

set -e

### 🔧 KONFIGURACJA ###
APP_NAME="Inwentaryzacja"
QT_PATH="/Users/Arek/Qt/6.5.3/macos"
DEPLOYQT="$QT_PATH/bin/macdeployqt"
APP_DIR="build/Qt_6_5_3_for_macOS-Release/${APP_NAME}.app"
DMG_NAME="${APP_NAME}_macOS.dmg"

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
mkdir -p tmp_dmg
cp -R "$APP_DIR" tmp_dmg/

hdiutil create -volname "$APP_NAME" -srcfolder tmp_dmg -ov -format UDZO "$DMG_NAME"
rm -rf tmp_dmg

echo "✅ Gotowe! Wygenerowano: $DMG_NAME"

