#!/bin/bash

set -e

APP_NAME="Inwentaryzacja"
DEPLOY_DIR="deploy"
BUILD_DIR="build_inwentaryzacja"
PLUGIN_BUILD_DIR="build_qt_sql_drivers"

# ============================
# Wczytanie QT_PATH z pliku, jeśli nie jest ustawiony
# ============================
if [[ -z "$QT_PATH" && -f qt_env.sh ]]; then
    echo "ℹ️  Wczytywanie QT_PATH z qt_env.sh"
    source qt_env.sh
fi

if [[ -z "$QT_PATH" ]]; then
    echo "❌ Zmienna QT_PATH nie jest ustawiona. Ustaw ją przed uruchomieniem skryptu lub stwórz qt_env.sh."
    exit 1
fi

QT_PLUGIN_DIR="$QT_PATH/plugins"

echo -e "\n🚀 Rozpoczynam deploy dla macOS..."

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
# Kopiowanie pluginów Qt
# ============================
echo "🔌 Kopiowanie pluginów Qt..."
cp "$QT_PLUGIN_DIR/platforms/libqcocoa.dylib" "$DEPLOY_DIR/platforms/" || echo "⚠️ Brak libqcocoa.dylib"
cp "$QT_PLUGIN_DIR/sqldrivers/libqsqlite.dylib" "$DEPLOY_DIR/sqldrivers/" || echo "⚠️ Brak libqsqlite.dylib"

# ============================
# Kopiowanie pluginu qsqlmysql (zbudowanego)
# ============================
if [[ -f "$PLUGIN_BUILD_DIR/plugins/sqldrivers/libqsqlmysql.dylib" ]]; then
    cp "$PLUGIN_BUILD_DIR/plugins/sqldrivers/libqsqlmysql.dylib" "$DEPLOY_DIR/sqldrivers/"
    echo "✅ Skopiowano libqsqlmysql.dylib"
else
    echo "❌ Nie znaleziono libqsqlmysql.dylib. Czy plugin został zbudowany?"
    exit 1
fi

# ============================
# Kopiowanie zależności binarki
# ============================
echo -e "\n📦 Kopiowanie zależności binarki..."
otool -L "$DEPLOY_DIR/$APP_NAME" | awk 'NR>1 {print $1}' | while read -r lib; do
    if [[ "$lib" == /usr/local/* || "$lib" == /opt/homebrew/* || "$lib" == "$QT_PATH"* ]]; then
        echo "📥 $lib"
        cp -u "$lib" "$DEPLOY_DIR/" 2>/dev/null || true
    fi
done

# ============================
# Kopiowanie zależności pluginu qsqlmysql
# ============================
echo -e "\n🔍 Kopiowanie zależności pluginu qsqlmysql.dylib..."
otool -L "$DEPLOY_DIR/sqldrivers/libqsqlmysql.dylib" | awk 'NR>1 {print $1}' | while read -r lib; do
    if [[ "$lib" == /usr/local/* || "$lib" == /opt/homebrew/* || "$lib" == "$QT_PATH"* ]]; then
        echo "📥 $lib"
        cp -u "$lib" "$DEPLOY_DIR/" 2>/dev/null || true
    fi
done

# ============================
# Gotowe
# ============================
echo -e "\n✅ Deploy zakończony: $DEPLOY_DIR/ zawiera wszystko do uruchomienia aplikacji"
echo "💡 Uruchom z lokalnymi pluginami Qt:"
echo "   cd $DEPLOY_DIR && QT_QPA_PLATFORM_PLUGIN_PATH=./platforms QT_PLUGIN_PATH=./sqldrivers ./$APP_NAME"
