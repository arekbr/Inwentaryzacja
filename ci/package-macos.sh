#!/bin/bash
set -euo pipefail

BUILD_DIR="${1:?Podaj katalog build}"
OUTPUT_DIR="${2:?Podaj katalog wyjściowy}"
ARCH_LABEL="${3:-macos}"

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
APP_NAME="Inwentaryzacja"
VERSION="$(tr -d '[:space:]' < "$ROOT_DIR/version.txt")"
QT_PATH="$(qmake -query QT_INSTALL_PREFIX)"
QT_PLUGINS="$(qmake -query QT_INSTALL_PLUGINS)"
MACDEPLOYQT="$QT_PATH/bin/macdeployqt"

BUILD_APP="$ROOT_DIR/$BUILD_DIR/$APP_NAME"
if [[ ! -f "$BUILD_APP" ]]; then
    BUILD_APP="$ROOT_DIR/$BUILD_DIR/Release/$APP_NAME"
fi

if [[ ! -f "$BUILD_APP" ]]; then
    echo "❌ Nie znaleziono binarki: $APP_NAME"
    exit 1
fi

STAGE_DIR="$ROOT_DIR/$OUTPUT_DIR/stage"
APP_BUNDLE="$STAGE_DIR/$APP_NAME.app"
APP_CONTENTS="$APP_BUNDLE/Contents"
APP_MACOS="$APP_CONTENTS/MacOS"
APP_RESOURCES="$APP_CONTENTS/Resources"
APP_PLUGINS="$APP_CONTENTS/PlugIns"
APP_SQL_DRIVERS="$APP_PLUGINS/sqldrivers"

rm -rf "$STAGE_DIR"
mkdir -p "$APP_MACOS" "$APP_RESOURCES" "$APP_SQL_DRIVERS" "$ROOT_DIR/$OUTPUT_DIR"

cp "$BUILD_APP" "$APP_MACOS/$APP_NAME"
chmod +x "$APP_MACOS/$APP_NAME"

if [[ -f "$ROOT_DIR/images/ikona_mac.icns" ]]; then
    cp "$ROOT_DIR/images/ikona_mac.icns" "$APP_RESOURCES/$APP_NAME.icns"
fi

if [[ -f "$QT_PLUGINS/sqldrivers/libqsqlite.dylib" ]]; then
    cp "$QT_PLUGINS/sqldrivers/libqsqlite.dylib" "$APP_SQL_DRIVERS/"
fi

if [[ -f "$QT_PLUGINS/sqldrivers/libqsqlmysql.dylib" ]]; then
    cp "$QT_PLUGINS/sqldrivers/libqsqlmysql.dylib" "$APP_SQL_DRIVERS/"
fi

cat > "$APP_CONTENTS/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDisplayName</key><string>$APP_NAME</string>
  <key>CFBundleExecutable</key><string>$APP_NAME</string>
  <key>CFBundleIdentifier</key><string>com.smoktechnology.$APP_NAME</string>
  <key>CFBundleName</key><string>$APP_NAME</string>
  <key>CFBundleVersion</key><string>$VERSION</string>
  <key>CFBundleShortVersionString</key><string>$VERSION</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleIconFile</key><string>$APP_NAME.icns</string>
</dict>
</plist>
EOF

"$MACDEPLOYQT" "$APP_BUNDLE" -verbose=1

ZIP_NAME="$ROOT_DIR/$OUTPUT_DIR/${APP_NAME}_${VERSION}_macOS_${ARCH_LABEL}.zip"
DMG_NAME="$ROOT_DIR/$OUTPUT_DIR/${APP_NAME}_${VERSION}_macOS_${ARCH_LABEL}.dmg"

rm -f "$ZIP_NAME" "$DMG_NAME"
ditto -c -k --sequesterRsrc --keepParent "$APP_BUNDLE" "$ZIP_NAME"
hdiutil create -volname "$APP_NAME" -srcfolder "$APP_BUNDLE" -ov -format UDZO "$DMG_NAME"

echo "✅ Gotowe pakiety:"
echo "   $ZIP_NAME"
echo "   $DMG_NAME"
