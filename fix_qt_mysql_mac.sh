#!/bin/bash

# ============================
# KONFIGURACJA
# ============================

MYSQL_LIB="/usr/local/mysql-8.0.31-macos12-x86_64/lib/libmysqlclient.21.dylib"
QT_PLUGIN_PATH="$HOME/Qt/6.5.3/macos/plugins/sqldrivers"

# Szukamy bibliotek SSL 1.1 (potrzebnych przez MySQL 8)
LIBSSL=$(find /usr/local /opt /usr -name "libssl.1.1.dylib" 2>/dev/null | head -n 1)
LIBCRYPTO=$(find /usr/local /opt /usr -name "libcrypto.1.1.dylib" 2>/dev/null | head -n 1)

# ============================
# SZUKAJ .app
# ============================

APP_PATH=$(find ./build -type d -name "*.app" | head -n 1)
if [[ -z "$APP_PATH" ]]; then
  echo "❌ Nie znaleziono .app w katalogu build/"
  exit 1
fi

APP_NAME=$(basename "$APP_PATH" .app)
BIN_PATH="$APP_PATH/Contents/MacOS/$APP_NAME"
PLUGIN_DIR="$APP_PATH/Contents/PlugIns/sqldrivers"
PLUGIN_PATH="$PLUGIN_DIR/libqsqlmysql.dylib"
FRAMEWORKS_PATH="$APP_PATH/Contents/Frameworks"

echo "📦 Aplikacja: $APP_PATH"
echo "⚙️ Binarka: $BIN_PATH"

# ============================
# Plugin QMYSQL
# ============================

mkdir -p "$PLUGIN_DIR"
if [[ ! -f "$PLUGIN_PATH" ]]; then
  echo "🔌 Kopiuję plugin QMYSQL..."
  cp "$QT_PLUGIN_PATH/libqsqlmysql.dylib" "$PLUGIN_PATH" || {
    echo "❌ Nie udało się skopiować pluginu Qt MySQL"
    exit 1
  }
else
  echo "✅ Plugin QMYSQL już obecny."
fi

# ============================
# libmysqlclient
# ============================

mkdir -p "$FRAMEWORKS_PATH"
if [[ ! -f "$FRAMEWORKS_PATH/libmysqlclient.21.dylib" ]]; then
  echo "📥 Kopiuję libmysqlclient.21.dylib..."
  cp "$MYSQL_LIB" "$FRAMEWORKS_PATH/"
else
  echo "✅ libmysqlclient.21.dylib już obecny."
fi

# ============================
# libssl i libcrypto
# ============================

if [[ -n "$LIBSSL" && -n "$LIBCRYPTO" ]]; then
  echo "🔐 Kopiuję libssl i libcrypto..."
  cp "$LIBSSL" "$FRAMEWORKS_PATH/"
  cp "$LIBCRYPTO" "$FRAMEWORKS_PATH/"
else
  echo "❌ Nie znaleziono libssl.1.1.dylib lub libcrypto.1.1.dylib"
  exit 1
fi

# ============================
# PATCH pluginu Qt
# ============================

echo "🛠️ Patchuję plugin QMYSQL..."
install_name_tool -change @rpath/libmysqlclient.21.dylib \
  @executable_path/../Frameworks/libmysqlclient.21.dylib \
  "$PLUGIN_PATH"

# ============================
# PATCH libmysqlclient (SSL deps)
# ============================

echo "🛠️ Patchuję libmysqlclient.21.dylib..."
install_name_tool -change @loader_path/../lib/libssl.1.1.dylib \
  @executable_path/../Frameworks/libssl.1.1.dylib \
  "$FRAMEWORKS_PATH/libmysqlclient.21.dylib"

install_name_tool -change @loader_path/../lib/libcrypto.1.1.dylib \
  @executable_path/../Frameworks/libcrypto.1.1.dylib \
  "$FRAMEWORKS_PATH/libmysqlclient.21.dylib"

# ============================
# RPATH (na wszelki wypadek)
# ============================

echo "🔧 Sprawdzam @rpath..."
if ! otool -l "$BIN_PATH" | grep -q "@executable_path/../Frameworks"; then
  echo "➕ Dodaję @rpath..."
  install_name_tool -add_rpath @executable_path/../Frameworks "$BIN_PATH"
else
  echo "ℹ️ @rpath już obecny."
fi

echo "✅ GOTOWE: Twoja aplikacja Qt powinna teraz bez problemu ładować driver QMYSQL + libssl."

