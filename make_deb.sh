#!/bin/bash
set -e

APP_NAME="Inwentaryzacja"
VERSION="1.2.0"
ARCH="amd64"

mkdir -p deploy/DEBIAN
cat <<EOF > deploy/DEBIAN/control
Package: $APP_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: $ARCH
Depends: libc6 (>= 2.31), libqt6core6, libqt6widgets6, libqt6gui6, libqt6sql6
Maintainer: Twoje Imię <twoj.email@example.com>
Description: Program do inwentaryzacji retro komputerów
EOF

dpkg-deb --build deploy "${APP_NAME}_${VERSION}_${ARCH}.deb"

echo "✅ Utworzono paczkę: ${APP_NAME}_${VERSION}_${ARCH}.deb"
