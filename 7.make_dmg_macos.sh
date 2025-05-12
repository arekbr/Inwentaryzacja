#!/bin/bash
set -e

APP_NAME="Inwentaryzacja"
APP_VERSION="1.2.4"
DEPLOY_DIR="deploy"
APP_PATH="$DEPLOY_DIR/$APP_NAME.app"
DMG_NAME="${APP_NAME}_${APP_VERSION}_macOS.dmg"

# ============================
# Wczytanie QT_PATH z qt_env.sh
# ============================
if [[ -z "$QT_PATH" && -f qt_env.sh ]]; then
    source qt_env.sh
fi

if [[ -z "$QT_PATH" ]]; then
    echo "❌ Zmienna QT_PATH nie jest ustawiona."
    exit 1
fi

echo -e "\n🚀 Rozpoczynam budowę .app i .dmg dla $APP_NAME"

# ============================
# ❗ Wcześniejsze czyszczenie niechcianych pluginów z katalogu Qt
# ============================
echo "🧹 Wstępne czyszczenie niechcianych pluginów z katalogu Qt..."
for plugin in qsqlodbc qsqlpsql qsqloci qsqlibm qsqlqdb2 qsqlmimer; do
    file="$QT_PATH/plugins/sqldrivers/lib${plugin}.dylib"
    if [[ -f "$file" ]]; then
        echo "   🔥 Usuwam z katalogu Qt: $file"
        rm -f "$file"
    fi
done

# ============================
# Budowa struktury .app
# ============================
echo "📁 Tworzenie struktury $APP_PATH"
rm -rf "$APP_PATH"
mkdir -p "$APP_PATH/Contents/MacOS"
mkdir -p "$APP_PATH/Contents/Resources"

cp "$DEPLOY_DIR/$APP_NAME" "$APP_PATH/Contents/MacOS/"

# ============================
# Dodanie ikony aplikacji
# ============================
ICON_SRC="images/ikona_mac.icns"
if [[ -f "$ICON_SRC" ]]; then
    cp "$ICON_SRC" "$APP_PATH/Contents/Resources/$APP_NAME.icns"
    echo "✅ Ikona aplikacji dodana"
else
    echo "⚠️ Brak ikony $ICON_SRC — pominięto"
fi

# ============================
# Tworzenie Info.plist
# ============================
cat > "$APP_PATH/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
 "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDisplayName</key><string>$APP_NAME</string>
  <key>CFBundleExecutable</key><string>$APP_NAME</string>
  <key>CFBundleIdentifier</key><string>com.smoktechnology.$APP_NAME</string>
  <key>CFBundleName</key><string>$APP_NAME</string>
  <key>CFBundleVersion</key><string>$APP_VERSION</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleIconFile</key><string>$APP_NAME.icns</string>
</dict>
</plist>
EOF

# ============================
# Uruchamianie macdeployqt
# ============================
echo "🔧 Uruchamianie macdeployqt..."
"$QT_PATH/bin/macdeployqt" "$APP_PATH" -verbose=2

# ============================
# Tworzenie DMG
# ============================
echo "📦 Tworzenie struktury DMG..."
rm -f "$DEPLOY_DIR/$DMG_NAME"
hdiutil create -volname "$APP_NAME" -srcfolder "$APP_PATH" -ov -format UDZO "$DEPLOY_DIR/$DMG_NAME"

echo -e "\n✅ Gotowe: $DEPLOY_DIR/$DMG_NAME"
echo "📦 Możesz teraz przetestować montowanie i uruchamianie .app z DMG"
