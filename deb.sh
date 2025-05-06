#!/bin/bash
set -e

APP_NAME="Inwentaryzacja"
VERSION="1.2.0"
ARCH="amd64"
DEPLOY_DIR="deploy"
INSTALL_PREFIX="/usr"
QT_LIB_DIR="/home/arekbr/Qt/6.8.2/gcc_64/lib"
PLUGIN_DIR="/home/arekbr/Qt/6.8.2/gcc_64/plugins/sqldrivers"
APP_BIN="$DEPLOY_DIR${INSTALL_PREFIX}/bin/$APP_NAME"

echo "📦 Czyszczenie i przygotowanie struktury paczki..."
rm -rf "$DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR/DEBIAN"
mkdir -p "$DEPLOY_DIR${INSTALL_PREFIX}/bin"
mkdir -p "$DEPLOY_DIR${INSTALL_PREFIX}/lib/$APP_NAME"
mkdir -p "$DEPLOY_DIR${INSTALL_PREFIX}/lib/$APP_NAME/plugins/sqldrivers"

echo "🛠️ Budowanie projektu (release)..."
qmake6 && make -j$(nproc)

echo "📂 Kopiowanie binarki..."
if [ ! -f "./$APP_NAME" ]; then
    echo "❌ Błąd: Nie znaleziono binarki ./$APP_NAME. Upewnij się, że kompilacja się powiodła."
    exit 1
fi
cp "./$APP_NAME" "$APP_BIN"

echo "📄 Tworzenie pliku control..."
cat <<EOF > "$DEPLOY_DIR/DEBIAN/control"
Package: $APP_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: $ARCH
Maintainer: Arkadiusz Bronowicki <arek@bronowicki.com>
Description: Program do inwentaryzacji retro komputerów
EOF

echo "🔍 Zbieranie zależności z ldd..."
ALL_LIBS=$(ldd "$APP_BIN" | awk '{if ($(NF-1) == "=>") print $3}' | grep "^/")

for LIB in $ALL_LIBS; do
    echo "📎 Kopiowanie: $LIB"
    cp -v --parents "$LIB" "$DEPLOY_DIR"
done

echo "🔍 Dodawanie zależności Qt spoza ldd (np. ICU)..."
for LIB in libicui18n.so.73 libicuuc.so.73 libicudata.so.73; do
    if [ -f "$QT_LIB_DIR/$LIB" ]; then
        cp "$QT_LIB_DIR/$LIB" "$DEPLOY_DIR${INSTALL_PREFIX}/lib/$APP_NAME/"
        echo "✅ Dodano $LIB"
    fi
done

echo "🧩 Dodawanie pluginu Qt MySQL..."
cp "$PLUGIN_DIR/libqsqlmysql.so" "$DEPLOY_DIR${INSTALL_PREFIX}/lib/$APP_NAME/plugins/sqldrivers/"

echo "🔍 Szukanie zależności pluginu MySQL..."
MYSQL_PLUGIN="$DEPLOY_DIR${INSTALL_PREFIX}/lib/$APP_NAME/plugins/sqldrivers/libqsqlmysql.so"
PLUGIN_LIBS=$(ldd "$MYSQL_PLUGIN" | awk '{if ($(NF-1) == "=>") print $3}' | grep "^/" || true)

for LIB in $PLUGIN_LIBS; do
    echo "📎 [plugin] Kopiowanie: $LIB"
    cp -v --parents "$LIB" "$DEPLOY_DIR"
done

echo "🔧 Ustawianie RPATH..."
patchelf --set-rpath "\$ORIGIN/../lib/$APP_NAME" "$APP_BIN"
patchelf --set-rpath "\$ORIGIN/../../lib/$APP_NAME" "$MYSQL_PLUGIN"

echo "🖼️ Dodawanie ikony i .desktop..."
ICON_NAME="$APP_NAME.png"
ICON_SRC="./icon.png"
ICON_DST_DIR="$DEPLOY_DIR/usr/share/icons/hicolor/128x128/apps"
DESKTOP_FILE="$DEPLOY_DIR/usr/share/applications/$APP_NAME.desktop"

mkdir -p "$ICON_DST_DIR"
mkdir -p "$(dirname "$DESKTOP_FILE")"

if [ -f "$ICON_SRC" ]; then
    cp "$ICON_SRC" "$ICON_DST_DIR/$ICON_NAME"
else
    echo "⚠️ Ostrzeżenie: Nie znaleziono $ICON_SRC — ikona nie zostanie dodana."
fi

cat <<EOF > "$DESKTOP_FILE"
[Desktop Entry]
Version=1.0
Name=$APP_NAME
Comment=Program do inwentaryzacji retro komputerów
Exec=${INSTALL_PREFIX}/bin/$APP_NAME
Icon=$APP_NAME
Terminal=false
Type=Application
Categories=Utility;
EOF

echo "📦 Tworzenie paczki DEB..."
dpkg-deb --build "$DEPLOY_DIR" "${APP_NAME}_${VERSION}_${ARCH}.deb"

echo "✅ Gotowe: ${APP_NAME}_${VERSION}_${ARCH}.deb"
