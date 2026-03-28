#!/bin/bash
set -euo pipefail

BUILD_DIR="${1:?Podaj katalog build}"
OUTPUT_DIR="${2:?Podaj katalog wyjściowy}"

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
APP_NAME="Inwentaryzacja"
VERSION="$(tr -d '[:space:]' < "$ROOT_DIR/version.txt")"
ARCH="amd64"
PACKAGE_NAME="inwentaryzacja"
STAGE_DIR="$ROOT_DIR/$OUTPUT_DIR/${PACKAGE_NAME}_${VERSION}_${ARCH}"

BUILD_APP="$ROOT_DIR/$BUILD_DIR/$APP_NAME"
if [[ ! -f "$BUILD_APP" ]]; then
    BUILD_APP="$ROOT_DIR/$BUILD_DIR/Release/$APP_NAME"
fi

if [[ ! -f "$BUILD_APP" ]]; then
    echo "❌ Nie znaleziono binarki: $APP_NAME"
    exit 1
fi

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR/DEBIAN"
mkdir -p "$STAGE_DIR/usr/bin"
mkdir -p "$STAGE_DIR/usr/share/applications"
mkdir -p "$STAGE_DIR/usr/share/icons/hicolor/256x256/apps"

cp "$BUILD_APP" "$STAGE_DIR/usr/bin/$APP_NAME"
chmod +x "$STAGE_DIR/usr/bin/$APP_NAME"

cat > "$STAGE_DIR/DEBIAN/control" <<EOF
Package: $PACKAGE_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: $ARCH
Maintainer: Arek Bronowicki <arek@bronowicki.com>
Depends: libqt6core6, libqt6dbus6, libqt6gui6, libqt6network6, libqt6sql6, libqt6widgets6, libqt6sql6-sqlite | libqt6sql6-mysql, libxcb-cursor0
Description: Program do inwentaryzacji zbiorów retro muzeum
 Aplikacja Qt do katalogowania eksponatów, zdjęć i metadanych.
EOF

cat > "$STAGE_DIR/usr/share/applications/$PACKAGE_NAME.desktop" <<EOF
[Desktop Entry]
Version=$VERSION
Type=Application
Name=Inwentaryzacja
GenericName=Retro Inventory
Comment=Program do inwentaryzacji zbiorów retro muzeum
Exec=/usr/bin/Inwentaryzacja
Icon=$PACKAGE_NAME
Categories=Utility;
Terminal=false
EOF

if [[ -f "$ROOT_DIR/images/icon.png" ]]; then
    cp "$ROOT_DIR/images/icon.png" "$STAGE_DIR/usr/share/icons/hicolor/256x256/apps/$PACKAGE_NAME.png"
fi

mkdir -p "$ROOT_DIR/$OUTPUT_DIR"
dpkg-deb --build "$STAGE_DIR"
PACKAGE_PATH="$ROOT_DIR/$OUTPUT_DIR/${PACKAGE_NAME}_${VERSION}_${ARCH}.deb"
if [[ "$STAGE_DIR.deb" != "$PACKAGE_PATH" ]]; then
    mv "$STAGE_DIR.deb" "$PACKAGE_PATH"
fi

echo "✅ Gotowe: $PACKAGE_PATH"
