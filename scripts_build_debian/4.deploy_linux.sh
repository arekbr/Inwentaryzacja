#!/bin/bash

set -e

APP_NAME="Inwentaryzacja"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
DEPLOY_DIR="$ROOT_DIR/deploy"
BUILD_DIR="$ROOT_DIR/build_inwentaryzacja"
PLUGIN_BUILD_DIR="$ROOT_DIR/build_qt_sql_drivers"

# ============================
# Wczytanie QT_PATH z pliku, jeśli nie jest ustawiony
# ============================
if [[ -z "$QT_PATH" && -f "$ROOT_DIR/qt_env.sh" ]]; then
    echo "ℹ️  Wczytywanie QT_PATH z qt_env.sh"
    source "$ROOT_DIR/qt_env.sh"
fi

if [[ -z "$QT_PATH" ]]; then
    echo "❌ Zmienna QT_PATH nie jest ustawiona. Ustaw ją przed uruchomieniem skryptu lub stwórz qt_env.sh."
    exit 1
fi

QT_LIB_DIR="$QT_PATH/lib"
QT_PLUGIN_DIR=""
if command -v qtpaths6 >/dev/null 2>&1; then
    QT_PLUGIN_DIR="$(qtpaths6 --plugin-dir)"
elif command -v qtpaths >/dev/null 2>&1; then
    QT_PLUGIN_DIR="$(qtpaths --plugin-dir)"
fi

if [[ -z "$QT_PLUGIN_DIR" ]]; then
    if [[ -d "$QT_PATH/plugins" ]]; then
        QT_PLUGIN_DIR="$QT_PATH/plugins"
    elif [[ -d "/usr/lib/x86_64-linux-gnu/qt6/plugins" ]]; then
        QT_PLUGIN_DIR="/usr/lib/x86_64-linux-gnu/qt6/plugins"
    else
        echo "❌ Nie znaleziono katalogu pluginow Qt."
        exit 1
    fi
fi


echo -e "\n🚀 Rozpoczynam deploy dla Linuxa..."

# ============================
# Czyszczenie i tworzenie katalogu deploy
# ============================
echo "🧹 Czyszczenie $DEPLOY_DIR..."
rm -rf "$DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR"

# ============================
# Kopiowanie binarki
# ============================
echo "📂 Kopiowanie binarki: $APP_NAME"
cp "$BUILD_DIR/$APP_NAME" "$DEPLOY_DIR/"

# ============================
# Tworzenie podkatalogów pluginów
# ============================
echo "📁 Tworzenie podkatalogów plugins..."
mkdir -p "$DEPLOY_DIR/platforms"
mkdir -p "$DEPLOY_DIR/sqldrivers"

# ============================
# Kopiowanie pluginów Qt (tylko z poprawnego QT_PATH)
# ============================
echo "🔌 Kopiowanie pluginów Qt..."
cp "$QT_PLUGIN_DIR/platforms/libqxcb.so" "$DEPLOY_DIR/platforms/" || echo "⚠️ Brak libqxcb.so"
cp "$QT_PLUGIN_DIR/sqldrivers/libqsqlite.so" "$DEPLOY_DIR/sqldrivers/" || echo "⚠️ Brak libqsqlite.so"

# ============================
# Kopiowanie pluginu qsqlmysql (zbudowanego)
# ============================
if [[ -f "$PLUGIN_BUILD_DIR/plugins/sqldrivers/libqsqlmysql.so" ]]; then
    cp "$PLUGIN_BUILD_DIR/plugins/sqldrivers/libqsqlmysql.so" "$DEPLOY_DIR/sqldrivers/"
    echo "✅ Skopiowano libqsqlmysql.so (z budowy)"
elif [[ -f "$QT_PLUGIN_DIR/sqldrivers/libqsqlmysql.so" ]]; then
    cp "$QT_PLUGIN_DIR/sqldrivers/libqsqlmysql.so" "$DEPLOY_DIR/sqldrivers/"
    echo "✅ Skopiowano libqsqlmysql.so (z systemu)"
else
    echo "❌ Nie znaleziono libqsqlmysql.so. Zainstaluj pakiet Qt SQL MySQL lub zbuduj plugin."
    exit 1
fi

# ============================
# Kopiowanie bibliotek Qt wymaganych przez aplikację
# ============================
echo "📦 Kopiowanie bibliotek Qt zależnych od binarki..."
ldd "$DEPLOY_DIR/$APP_NAME" | awk '{print $3}' | while read -r lib; do
    if [[ -f "$lib" ]]; then
        echo "📥 $lib"
        cp -u "$lib" "$DEPLOY_DIR/"
        if [[ -L "$lib" ]]; then
            target=$(readlink -f "$lib")
            cp -u "$target" "$DEPLOY_DIR/"
        fi
    fi

done

# ============================
# Kopiowanie zależności pluginu qsqlmysql (libmariadb, libssl, libcrypto)
# ============================
echo -e "\n🔍 Kopiowanie zależności pluginu qsqlmysql.so..."
ldd "$DEPLOY_DIR/sqldrivers/libqsqlmysql.so" | awk '{print $3}' | while read -r lib; do
    if [[ -f "$lib" ]]; then
        echo "📥 $lib"
        cp -u "$lib" "$DEPLOY_DIR/"
        if [[ -L "$lib" ]]; then
            target=$(readlink -f "$lib")
            cp -u "$target" "$DEPLOY_DIR/"
        fi
    fi

done

# ============================
# Kopiowanie pluginów imageformats
# ============================
echo "🖼️  Kopiowanie pluginów imageformats..."
mkdir -p "$DEPLOY_DIR/imageformats"
cp "$QT_PLUGIN_DIR/imageformats/"* "$DEPLOY_DIR/imageformats/" 2>/dev/null || echo "⚠️  Brak pluginów imageformats"

# ============================
# Gotowe
# ============================
echo -e "\n✅ Deploy zakończony: $DEPLOY_DIR/ zawiera wszystko do uruchomienia aplikacji"
echo "💡 Uruchom z lokalnymi bibliotekami:"
echo "   cd $DEPLOY_DIR && QT_QPA_PLATFORM_PLUGIN_PATH=./platforms QT_PLUGIN_PATH=./sqldrivers LD_LIBRARY_PATH=.:$QT_PATH/lib ./$APP_NAME"
