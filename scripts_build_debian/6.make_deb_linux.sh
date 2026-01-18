#!/bin/bash

set -e

APP_NAME="Inwentaryzacja"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
DEPLOY_DIR="$ROOT_DIR/deploy"
DEB_DIR="$ROOT_DIR/deb_pkg"
# Odczytanie wersji z pliku version.txt
VERSION=$(cat "$ROOT_DIR/version.txt" | tr -d '[:space:]')
ARCH="amd64"
MAINTAINER="Stowarzyszenie SMOK <kontakt@smok.technology>"
DESCRIPTION="Program do inwentaryzacji retro komputerów"

# ============================
# Wczytanie QT_PATH z pliku qt_env.sh
# ============================
if [[ -z "$QT_PATH" && -f "$ROOT_DIR/qt_env.sh" ]]; then
    source "$ROOT_DIR/qt_env.sh"
fi

if [[ -z "$QT_PATH" ]]; then
    echo "❌ QT_PATH nie jest ustawiony."
    exit 1
fi

# ============================
# Tworzenie struktury katalogów
# ============================
echo "📦 Tworzenie struktury pakietu .deb"
rm -rf "$DEB_DIR"
mkdir -p "$DEB_DIR/DEBIAN"
mkdir -p "$DEB_DIR/usr/bin"
mkdir -p "$DEB_DIR/usr/share/$APP_NAME/platforms"
mkdir -p "$DEB_DIR/usr/share/$APP_NAME/sqldrivers"
mkdir -p "$DEB_DIR/usr/share/$APP_NAME/resources"
mkdir -p "$DEB_DIR/usr/share/applications"
mkdir -p "$DEB_DIR/usr/share/icons/hicolor/256x256/apps"

# ============================
# Plik kontrolny
# ============================
cat > "$DEB_DIR/DEBIAN/control" <<EOF
Package: $APP_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: $ARCH
Depends: libxcb-cursor0, libmariadb3, libssl3, libqt6core6, libqt6gui6, libqt6widgets6, libqt6sql6, libqt6dbus6
Maintainer: $MAINTAINER
Description: $DESCRIPTION
EOF

# ============================
# Kopiowanie plików aplikacji i środowiska
# ============================
cp "$DEPLOY_DIR/$APP_NAME" "$DEB_DIR/usr/bin/$APP_NAME-bin"
cp -r "$DEPLOY_DIR/platforms"/* "$DEB_DIR/usr/share/$APP_NAME/platforms/"
cp -r "$DEPLOY_DIR/sqldrivers"/* "$DEB_DIR/usr/share/$APP_NAME/sqldrivers/"
cp -u "$DEPLOY_DIR"/*.so* "$DEB_DIR/usr/share/$APP_NAME/" || true
cp "$ROOT_DIR/qt_env.sh" "$DEB_DIR/usr/share/$APP_NAME/qt_env.sh"

# ============================
# Kopiowanie pluginów imageformats (np. libqjpeg, libqpng)
# ============================
if [[ -d "$DEPLOY_DIR/imageformats" ]]; then
    mkdir -p "$DEB_DIR/usr/share/$APP_NAME/imageformats"
    cp -r "$DEPLOY_DIR/imageformats"/* "$DEB_DIR/usr/share/$APP_NAME/imageformats/"
else
    echo "⚠️  Brak katalogu $DEPLOY_DIR/imageformats – pluginy Qt imageformats nie zostaną dodane do .deb"
fi

# ============================
# Wrapper uruchamiający z exportami z qt_env.sh
# ============================
cat > "$DEB_DIR/usr/bin/$APP_NAME" <<'EOF'
#!/bin/bash

QT_ENV_PATH="/usr/share/Inwentaryzacja/qt_env.sh"
if [[ -f "$QT_ENV_PATH" ]]; then
    source "$QT_ENV_PATH"
else
    echo "❌ Nie znaleziono $QT_ENV_PATH — Qt może się nie uruchomić poprawnie" >&2
fi

export QT_QPA_PLATFORM_PLUGIN_PATH=/usr/share/Inwentaryzacja/platforms
export QT_PLUGIN_PATH=/usr/share/Inwentaryzacja
export LD_LIBRARY_PATH=/usr/share/Inwentaryzacja:${QT_PATH:+$QT_PATH/lib}:$LD_LIBRARY_PATH

exec /usr/bin/Inwentaryzacja-bin "$@"
EOF
chmod +x "$DEB_DIR/usr/bin/$APP_NAME"

# ============================
# Plik .desktop (GUI + log awaryjny)
# ============================
cat > "$DEB_DIR/usr/share/applications/$APP_NAME.desktop" <<EOF
[Desktop Entry]
Version=$VERSION
Type=Application
Name=Inwentaryzacja
GenericName=Retro Inventory
Comment=$DESCRIPTION
Exec=sh -c "/usr/bin/Inwentaryzacja >> /tmp/inwentaryzacja.log 2>&1"
Icon=$APP_NAME
Categories=Utility;
Terminal=false
EOF

# ============================
# Kopiowanie ikony
# ============================
if [[ -f "$ROOT_DIR/images/icon.png" ]]; then
    cp "$ROOT_DIR/images/icon.png" "$DEB_DIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png"
else
    echo "⚠️  Ostrzeżenie: Brak pliku ikony: images/icon.png"
fi

# ============================
# Budowa pakietu .deb
# ============================
echo "📦 Budowanie .deb..."
dpkg-deb --build --root-owner-group "$DEB_DIR"
mv "$DEB_DIR.deb" "${APP_NAME}_${VERSION}_${ARCH}.deb"
echo "✅ Gotowe: ${APP_NAME}_${VERSION}_${ARCH}.deb"
