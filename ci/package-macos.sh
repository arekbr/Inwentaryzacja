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
DMG_STAGE_DIR="$STAGE_DIR/dmg"
TEMP_ROOT="$(mktemp -d "/tmp/${APP_NAME}_${ARCH_LABEL}_XXXXXX")"
cleanup() {
    rm -rf "$TEMP_ROOT"
}
trap cleanup EXIT

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

# Plugin QMYSQL z kroku „Build QMYSQL Qt plugin” — bez niego paczka nie łączy się z MySQL,
# więc brak to błąd pakowania, nie cichy pominięty plik.
if [[ ! -f "$QT_PLUGINS/sqldrivers/libqsqlmysql.dylib" ]]; then
    echo "❌ Brak pluginu QMYSQL: $QT_PLUGINS/sqldrivers/libqsqlmysql.dylib"
    exit 1
fi
cp "$QT_PLUGINS/sqldrivers/libqsqlmysql.dylib" "$APP_SQL_DRIVERS/"

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

# Po macdeployqt plugin QMYSQL ma wskazywać libmariadb W paczce — ścieżka do brew z runnera
# u usera nie istnieje (sonda w CI sprawdza plugin w Qt, nie w paczce).
MARIADB_DEP="$(otool -L "$APP_SQL_DRIVERS/libqsqlmysql.dylib" | awk '/libmariadb/ {print $1; exit}')"
echo "▶ QMYSQL → $MARIADB_DEP"
if [[ "$MARIADB_DEP" != "@executable_path/../Frameworks/"* \
      || ! -f "$APP_CONTENTS/Frameworks/${MARIADB_DEP##*/}" ]]; then
    echo "❌ Plugin QMYSQL w paczce nie ma swojej biblioteki libmariadb w Contents/Frameworks."
    exit 1
fi

# Tożsamość Developer ID z CI (ci/import-macos-codesign.sh). Bez niej podpis ad hoc (budowa lokalna,
# PR z forka); wydanie (REQUIRE_CODESIGN=1) wymaga tożsamości.
SIGN_IDENTITY="${MACOS_CODESIGN_IDENTITY:--}"
if [[ "$SIGN_IDENTITY" == "-" && "${REQUIRE_CODESIGN:-0}" == "1" ]]; then
    echo "❌ Brak tożsamości podpisu (MACOS_CODESIGN_IDENTITY) — wydanie wymaga stałego podpisu."
    exit 1
fi

# Notaryzację włącza workflow (NOTARIZE=1 przy tagu i ręcznym starcie). Bez kompletu danych to błąd,
# a nie paczka, która po cichu nie przeszła notaryzacji.
NOTARIZE="${NOTARIZE:-0}"
if [[ "$NOTARIZE" == "1" ]]; then
    if [[ "$SIGN_IDENTITY" == "-" || -z "${APPLE_NOTARY_KEY_P8_BASE64:-}" \
          || -z "${APPLE_NOTARY_KEY_ID:-}" || -z "${APPLE_NOTARY_ISSUER_ID:-}" ]]; then
        echo "❌ Notaryzacja wymaga tożsamości podpisu i sekretów APPLE_NOTARY_*."
        exit 1
    fi
    NOTARY_KEY="$TEMP_ROOT/notary-key.p8"
    (umask 077 && printf '%s' "$APPLE_NOTARY_KEY_P8_BASE64" | base64 --decode > "$NOTARY_KEY")
fi

notarize_file() {
    local file="$1" result status submission
    result="$(xcrun notarytool submit "$file" --key "$NOTARY_KEY" --key-id "$APPLE_NOTARY_KEY_ID" \
        --issuer "$APPLE_NOTARY_ISSUER_ID" --wait --timeout 30m --output-format json)"
    status="$(printf '%s' "$result" | python3 -c 'import json,sys; print(json.load(sys.stdin).get("status",""))')"
    submission="$(printf '%s' "$result" | python3 -c 'import json,sys; print(json.load(sys.stdin).get("id",""))')"
    echo "▶ Notaryzacja ${file##*/}: $status ($submission)"
    if [[ "$status" != "Accepted" ]]; then
        xcrun notarytool log "$submission" --key "$NOTARY_KEY" --key-id "$APPLE_NOTARY_KEY_ID" \
            --issuer "$APPLE_NOTARY_ISSUER_ID" || true
        echo "❌ Apple nie przyjęło ${file##*/} do notaryzacji."
        exit 1
    fi
}

if [[ "$SIGN_IDENTITY" == "-" ]]; then
    codesign --force --deep --sign - --timestamp=none "$APP_BUNDLE"
else
    # Od środka na zewnątrz: notaryzacja wymaga wzmocnionego uruchamiania i znacznika czasu na każdym
    # składniku, a --deep nie gwarantuje tego dla frameworków.
    sign_for_release() {
        codesign --force --sign "$SIGN_IDENTITY" --options runtime --timestamp "$1"
    }
    while IFS= read -r -d '' library; do
        sign_for_release "$library"
    done < <(find "$APP_CONTENTS" -type f -name '*.dylib' -print0)
    if [[ -d "$APP_CONTENTS/Frameworks" ]]; then
        while IFS= read -r -d '' framework; do
            sign_for_release "$framework"
        done < <(find "$APP_CONTENTS/Frameworks" -type d -name '*.framework' -prune -print0)
    fi
    sign_for_release "$APP_BUNDLE"
fi
codesign --verify --deep --strict "$APP_BUNDLE"
DESIGNATED="$(codesign -d -r- "$APP_BUNDLE" 2>&1 | grep 'designated' || true)"
echo "▶ $DESIGNATED"
if [[ "$SIGN_IDENTITY" != "-" && "$DESIGNATED" != *"certificate "* ]]; then
    echo "❌ Wymaganie podpisu nie opiera się na certyfikacie — podpis tożsamością z CI nie zadziałał."
    exit 1
fi

# Zatwierdzenie dołączamy do aplikacji przed spakowaniem, żeby ZIP i DMG zawierały je bez sieci.
if [[ "$NOTARIZE" == "1" ]]; then
    NOTARY_ZIP="$TEMP_ROOT/notary-app.zip"
    ditto -c -k --keepParent "$APP_BUNDLE" "$NOTARY_ZIP"
    notarize_file "$NOTARY_ZIP"
    xcrun stapler staple "$APP_BUNDLE"
    xcrun stapler validate "$APP_BUNDLE"
    spctl --assess --type execute --verbose=4 "$APP_BUNDLE"
fi

ZIP_NAME="$ROOT_DIR/$OUTPUT_DIR/${APP_NAME}_${VERSION}_macOS_${ARCH_LABEL}.zip"
DMG_NAME="$ROOT_DIR/$OUTPUT_DIR/${APP_NAME}_${VERSION}_macOS_${ARCH_LABEL}.dmg"
DMG_VOLUME_NAME="${APP_NAME}-${ARCH_LABEL}${GITHUB_RUN_ID:+-${GITHUB_RUN_ID}}"
TEMP_DMG_NAME="$TEMP_ROOT/${APP_NAME}_${VERSION}_macOS_${ARCH_LABEL}.dmg"

rm -f "$ZIP_NAME" "$DMG_NAME" "$TEMP_DMG_NAME"
ditto -c -k --sequesterRsrc --keepParent "$APP_BUNDLE" "$ZIP_NAME"
rm -rf "$DMG_STAGE_DIR"
mkdir -p "$DMG_STAGE_DIR"
cp -R "$APP_BUNDLE" "$DMG_STAGE_DIR/"
ln -s /Applications "$DMG_STAGE_DIR/Applications"

hdiutil detach "/Volumes/$DMG_VOLUME_NAME" -force >/dev/null 2>&1 || true
hdiutil create -volname "$DMG_VOLUME_NAME" -srcfolder "$DMG_STAGE_DIR" -ov -format UDZO "$TEMP_DMG_NAME"
hdiutil verify "$TEMP_DMG_NAME"
mv "$TEMP_DMG_NAME" "$DMG_NAME"
hdiutil verify "$DMG_NAME"

if [[ "$NOTARIZE" == "1" ]]; then
    codesign --force --sign "$SIGN_IDENTITY" --timestamp "$DMG_NAME"
    notarize_file "$DMG_NAME"
    xcrun stapler staple "$DMG_NAME"
    xcrun stapler validate "$DMG_NAME"
    spctl --assess --type open --context context:primary-signature --verbose=4 "$DMG_NAME"
fi

echo "✅ Gotowe pakiety:"
echo "   $ZIP_NAME"
echo "   $DMG_NAME"
