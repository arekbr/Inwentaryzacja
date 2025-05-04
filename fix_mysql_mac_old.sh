#!/bin/bash

# ============================
# KONFIGURACJA
# ============================

LIB_DIR="/Users/Arek/projektyQT/Inwentaryzacja/macos_lib_sql"
MYSQL_LIB="$LIB_DIR/libmysqlclient.21.dylib"
QT_PLUGIN_PATH="$LIB_DIR"
LIBSSL="$LIB_DIR/libssl.1.1.dylib"
LIBCRYPTO="$LIB_DIR/libcrypto.1.1.dylib"

# Sprawdzenie, czy wszystkie pliki istnieją
for lib in "$MYSQL_LIB" "$QT_PLUGIN_PATH/libqsqlmysql.dylib" "$LIBSSL" "$LIBCRYPTO"; do
  if [[ ! -f "$lib" ]]; then
    echo "❌ Brak pliku: $lib"
    exit 1
  fi
done

# ============================
# SPRAWDZENIE ARGUMENTU
# ============================

if [[ -z "$1" ]]; then
  echo "❌ Podaj ścieżkę do .app jako argument"
  exit 1
fi

APP_PATH="$1"
if [[ ! -d "$APP_PATH" ]]; then
  echo "❌ Podana ścieżka nie wskazuje na katalog .app: $APP_PATH"
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
  echo "🔌 Kopiuję plugin QMYSQL z $QT_PLUGIN_PATH/libqsqlmysql.dylib..."
  cp "$QT_PLUGIN_PATH/libqsqlmysql.dylib" "$PLUGIN_PATH" || {
    echo "❌ Nie udało się skopiować pluginu Qt MySQL"
    exit 1
  }
else
  echo "✅ Plugin QMYSQL już obecny w $PLUGIN_PATH."
fi

# ============================
# libmysqlclient
# ============================

mkdir -p "$FRAMEWORKS_PATH"
if [[ ! -f "$FRAMEWORKS_PATH/libmysqlclient.21.dylib" ]]; then
  echo "📥 Kopiuję $MYSQL_LIB do $FRAMEWORKS_PATH/libmysqlclient.21.dylib..."
  cp "$MYSQL_LIB" "$FRAMEWORKS_PATH/libmysqlclient.21.dylib" || {
    echo "❌ Błąd podczas kopiowania libmysqlclient.21.dylib"
    exit 1
  }
else
  echo "✅ libmysqlclient.21.dylib już obecny w $FRAMEWORKS_PATH."
fi

# ============================
# libssl i libcrypto
# ============================

echo "🔐 Kopiuję libssl i libcrypto..."
cp "$LIBSSL" "$FRAMEWORKS_PATH/libssl.1.1.dylib" || {
  echo "❌ Błąd podczas kopiowania libssl.1.1.dylib"
  exit 1
}
cp "$LIBCRYPTO" "$FRAMEWORKS_PATH/libcrypto.1.1.dylib" || {
  echo "❌ Błąd podczas kopiowania libcrypto.1.1.dylib"
  exit 1
}
echo "✅ libssl i libcrypto skopiowane."

# ============================
# PATCH pluginu Qt
# ============================

echo "🛠️ Patchuję plugin QMYSQL..."
install_name_tool -change @rpath/libmysqlclient.21.dylib \
  @executable_path/../Frameworks/libmysqlclient.21.dylib \
  "$PLUGIN_PATH" || {
    echo "❌ Błąd podczas patchowania pluginu QMYSQL"
    exit 1
  }

# ============================
# PATCH libmysqlclient (SSL deps)
# ============================

echo "🛠️ Patchuję libmysqlclient.21.dylib..."
install_name_tool -change @loader_path/../lib/libssl.1.1.dylib \
  @executable_path/../Frameworks/libssl.1.1.dylib \
  "$FRAMEWORKS_PATH/libmysqlclient.21.dylib" || {
    echo "❌ Błąd podczas patchowania libmysqlclient dla libssl"
    exit 1
  }

install_name_tool -change @loader_path/../lib/libcrypto.1.1.dylib \
  @executable_path/../Frameworks/libcrypto.1.1.dylib \
  "$FRAMEWORKS_PATH/libmysqlclient.21.dylib" || {
    echo "❌ Błąd podczas patchowania libmysqlclient dla libcrypto"
    exit 1
  }

# ============================
# RPATH (na wszelki wypadek)
# ============================

echo "🔧 Sprawdzam @rpath..."
if ! otool -l "$BIN_PATH" | grep -q "@executable_path/../Frameworks"; then
  echo "➕ Dodaję @rpath..."
 _neighboring_id="e77490b0-5a60-4781-81d1-1aa8a359b376" install_name_tool -add_rpath @executable_path/../Frameworks "$BIN_PATH" || {
    echo "❌ Błąd podczas dodawania @rpath"
    exit 1
  }
else
  echo "ℹ️ @rpath już obecny."
fi

echo "✅ GOTOWE: Biblioteki MySQL zostały skopiowane i spatchowane w $APP_PATH."
