#!/bin/bash

# Skrypt patchuje biblioteki MariaDB i OpenSSL w aplikacji Inwentaryzacja.app
# dla macOS, zapewniając poprawne zależności w folderze Frameworks.
# Kopiuje libqsqlmysql.dylib do PlugIns/sqldrivers.
# Dostosowany do MariaDB 11.7.2, OpenSSL 3.5.0, Qt 6.9.0.

# ============================
# KONFIGURACJA
# ============================

# Ścieżka do aplikacji (.app)
if [[ -z "$1" ]]; then
    echo "❌ Błąd: Podaj ścieżkę do Inwentaryzacja.app jako argument"
    exit 1
fi
APP_PATH="$1"

# Ścieżki do folderów w aplikacji
FRAMEWORKS_PATH="$APP_PATH/Contents/Frameworks"
PLUGINS_PATH="$APP_PATH/Contents/PlugIns/sqldrivers"
LIB_DIR="$HOME/projektyQT/Inwentaryzacja/mac_lib_sql"

# Biblioteki
LIBQSQLMYSQL="$PLUGINS_PATH/libqsqlmysql.dylib"
LIBMARIADB="$FRAMEWORKS_PATH/libmariadb.3.dylib"
LIBSSL="$FRAMEWORKS_PATH/libssl.3.dylib"
LIBCRYPTO="$FRAMEWORKS_PATH/libcrypto.3.dylib"

# Źródła bibliotek
LIBQSQLMYSQL_SRC="$LIB_DIR/sqldrivers/libqsqlmysql.dylib"
LIBMARIADB_SRC="$LIB_DIR/libmariadb.3.dylib"
LIBSSL_SRC="$LIB_DIR/libssl.3.dylib"
LIBCRYPTO_SRC="$LIB_DIR/libcrypto.3.dylib"

# ============================
# FUNKCJE
# ============================

# Sprawdza, czy plik istnieje i jest dostępny
check_file() {
    local file="$1"
    if [[ ! -f "$file" ]]; then
        echo "❌ Błąd: Plik nie istnieje: $file"
        exit 1
    fi
    if [[ ! -r "$file" || ! -w "$file" ]]; then
        echo "⚠️ Ostrzeżenie: Plik $file ma ograniczone uprawnienia, naprawiam..."
        chmod u+rw "$file" || {
            echo "❌ Błąd: Nie udało się nadać uprawnień do $file"
            exit 1
        }
    fi
}

# Kopiuje bibliotekę i nadaje uprawnienia
copy_library() {
    local src="$1"
    local dest="$2"
    echo "📥 Kopiuję $src -> $dest"
    cp "$src" "$dest" || {
        echo "❌ Błąd: Nie udało się skopiować $src do $dest"
        exit 1
    }
    echo "🔐 Nadaję uprawnienia rw-r--r-- dla $dest"
    chmod 644 "$dest" || {
        echo "❌ Błąd: Nie udało się nadać uprawnień do $dest"
        exit 1
    }
}

# Patchuje zależności biblioteki
patch_library() {
    local lib="$1"
    local old_path="$2"
    local new_path="$3"
    echo "🔧 Patchuję $lib: $old_path -> $new_path"
    install_name_tool -change "$old_path" "$new_path" "$lib" || {
        echo "❌ Błąd: Nie udało się spatchować $lib (sprawdź uprawnienia)"
        ls -l "$lib"
        exit 1
    }
}

# ============================
# GŁÓWNA LOGIKA
# ============================

echo "🚀 Rozpoczynam patchowanie bibliotek w $APP_PATH"

# Sprawdzenie, czy aplikacja istnieje
check_file "$APP_PATH/Contents/MacOS/Inwentaryzacja"

# Sprawdzenie źródłowych bibliotek
check_file "$LIBQSQLMYSQL_SRC"
check_file "$LIBMARIADB_SRC"
check_file "$LIBSSL_SRC"
check_file "$LIBCRYPTO_SRC"

# Tworzenie folderów Frameworks i PlugIns/sqldrivers
mkdir -p "$FRAMEWORKS_PATH" || {
    echo "❌ Błąd: Nie udało się utworzyć $FRAMEWORKS_PATH"
    exit 1
}
mkdir -p "$PLUGINS_PATH" || {
    echo "❌ Błąd: Nie udało się utworzyć $PLUGINS_PATH"
    exit 1
}
chmod u+rwx "$FRAMEWORKS_PATH" "$PLUGINS_PATH" || {
    echo "❌ Błąd: Nie udało się nadać uprawnień do folderów"
    exit 1
}

# Kopiowanie bibliotek
copy_library "$LIBQSQLMYSQL_SRC" "$LIBQSQLMYSQL"
copy_library "$LIBMARIADB_SRC" "$LIBMARIADB"
copy_library "$LIBSSL_SRC" "$LIBSSL"
copy_library "$LIBCRYPTO_SRC" "$LIBCRYPTO"

# Patchowanie libqsqlmysql.dylib
patch_library "$LIBQSQLMYSQL" \
    "/usr/local/Cellar/mariadb/11.7.2/lib/libmariadb.3.dylib" \
    "@executable_path/../Frameworks/libmariadb.3.dylib"

# Patchowanie libmariadb.3.dylib
patch_library "$LIBMARIADB" \
    "/usr/local/Cellar/openssl@3/3.5.0/lib/libssl.3.dylib" \
    "@executable_path/../Frameworks/libssl.3.dylib"
patch_library "$LIBMARIADB" \
    "/usr/local/Cellar/openssl@3/3.5.0/lib/libcrypto.3.dylib" \
    "@executable_path/../Frameworks/libcrypto.3.dylib"

# Weryfikacja zależności
echo "📋 Weryfikacja zależności $LIBQSQLMYSQL:"
otool -L "$LIBQSQLMYSQL"
echo "📋 Weryfikacja zależności $LIBMARIADB:"
otool -L "$LIBMARIADB"

echo "✅ GOTOWE: Biblioteki w $APP_PATH zostały spatchowane"
