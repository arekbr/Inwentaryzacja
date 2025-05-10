#!/bin/bash

# Skrypt kopiuje potrzebne biblioteki i katalogi z ich źródeł do folderu projektu
# mac_lib_sql w ~/projektyQT/Inwentaryzacja dla Qt 6.9.0, MariaDB 11.7.2, OpenSSL 3.5.0

# ============================
# KONFIGURACJA
# ============================

# Ścieżki źródłowe
QT_SRC_DIR="$HOME/Qt/6.9.0/macos/plugins"
MARIADB_SRC_DIR="/usr/local/Cellar/mariadb/11.7.2/lib"
OPENSSL_SRC_DIR="/usr/local/Cellar/openssl@3/3.5.0/lib"

# Ścieżka docelowa
PROJECT_LIB_DIR="$HOME/projektyQT/Inwentaryzacja/mac_lib_sql"

# Lista bibliotek: każda linia to "ścieżka_docelowa:ścieżka_źródłowa"
LIBS=(
    "iconengines/libqsvgicon.dylib:$QT_SRC_DIR/iconengines/libqsvgicon.dylib"
    "imageformats/libqgif.dylib:$QT_SRC_DIR/imageformats/libqgif.dylib"
    "imageformats/libqicns.dylib:$QT_SRC_DIR/imageformats/libqicns.dylib"
    "imageformats/libqico.dylib:$QT_SRC_DIR/imageformats/libqico.dylib"
    "imageformats/libqjpeg.dylib:$QT_SRC_DIR/imageformats/libqjpeg.dylib"
    "imageformats/libqmacheif.dylib:$QT_SRC_DIR/imageformats/libqmacheif.dylib"
    "imageformats/libqmacjp2.dylib:$QT_SRC_DIR/imageformats/libqmacjp2.dylib"
    "imageformats/libqpdf.dylib:$QT_SRC_DIR/imageformats/libqpdf.dylib"
    "imageformats/libqtga.dylib:$QT_SRC_DIR/imageformats/libqtga.dylib"
    "imageformats/libqtiff.dylib:$QT_SRC_DIR/imageformats/libqtiff.dylib"
    "imageformats/libqwbmp.dylib:$QT_SRC_DIR/imageformats/libqwbmp.dylib"
    "imageformats/libqwebp.dylib:$QT_SRC_DIR/imageformats/libqwebp.dylib"
    "libcrypto.3.dylib:$OPENSSL_SRC_DIR/libcrypto.3.dylib"
    "libmariadb.3.dylib:$MARIADB_SRC_DIR/libmariadb.3.dylib"
    "libqsqlite.dylib:$QT_SRC_DIR/sqldrivers/libqsqlite.dylib"
    "libqsqlmysql.dylib:$QT_SRC_DIR/sqldrivers/libqsqlmysql.dylib"
    "libssl.3.dylib:$OPENSSL_SRC_DIR/libssl.3.dylib"
    "platforms/libqcocoa.dylib:$QT_SRC_DIR/platforms/libqcocoa.dylib"
    "sqldrivers/libqsqlmysql.dylib:$QT_SRC_DIR/sqldrivers/libqsqlmysql.dylib"
    "sqldrivers/libqsqlite.dylib:$QT_SRC_DIR/sqldrivers/libqsqlite.dylib"
    "styles/libqmacstyle.dylib:$QT_SRC_DIR/styles/libqmacstyle.dylib"
)

# ============================
# FUNKCJE
# ============================

# Sprawdza, czy plik źródłowy istnieje
check_file() {
    local src="$1"
    if [[ ! -f "$src" ]]; then
        echo "❌ Błąd: Plik źródłowy nie istnieje: $src"
        return 1
    fi
    if [[ ! -r "$src" || ! -w "$src" ]]; then
        echo "⚠️ Ostrzeżenie: Plik źródłowy $src ma ograniczone uprawnienia, naprawiam..."
        chmod u+rw "$src" || {
            echo "❌ Błąd: Nie udało się nadać uprawnień do $src"
            exit 1
        }
    fi
    return 0
}

# Tworzy katalog docelowy, jeśli nie istnieje
create_dir() {
    local dir="$1"
    if [[ ! -d "$dir" ]]; then
        echo "📁 Tworzę katalog: $dir"
        mkdir -p "$dir" || {
            echo "❌ Błąd: Nie udało się utworzyć katalogu: $dir"
            exit 1
        }
    fi
    chmod u+rwx "$dir" || {
        echo "❌ Błąd: Nie udało się nadać uprawnień do $dir"
        exit 1
    }
}

# Kopiuje plik z źródła do celu
copy_file() {
    local src="$1"
    local dest="$2"
    # Usuń istniejący plik docelowy, jeśli istnieje i ma złe uprawnienia
    if [[ -f "$dest" ]]; then
        if [[ ! -w "$dest" ]]; then
            echo "⚠️ Plik docelowy $dest jest tylko do odczytu, naprawiam..."
            chmod u+w "$dest" || {
                echo "❌ Błąd: Nie udało się nadać uprawnień do $dest"
                exit 1
            }
        fi
        rm -f "$dest" || {
            echo "❌ Błąd: Nie udało się usunąć istniejącego pliku $dest"
            exit 1
        }
    fi
    echo "📥 Kopiuję $src -> $dest"
    cp "$src" "$dest" || {
        echo "❌ Błąd: Nie udało się skopiować pliku: $src do $dest"
        exit 1
    }
    echo "🔐 Nadaję uprawnienia rw-r--r-- dla $dest"
    chmod 644 "$dest" || {
        echo "❌ Błąd: Nie udało się nadać uprawnień do $dest"
        exit 1
    }
}

# ============================
# GŁÓWNA LOGIKA
# ============================

echo "🚀 Rozpoczynam kopiowanie bibliotek do $PROJECT_LIB_DIR"

# Sprawdzenie, czy folder projektu istnieje
create_dir "$PROJECT_LIB_DIR"

# Licznik błędów
ERROR_COUNT=0

# Kopiowanie bibliotek
for lib in "${LIBS[@]}"; do
    # Rozdziel ścieżkę docelową i źródłową
    dest_path="${lib%%:*}"
    src_path="${lib#*:}"
    
    # Sprawdzenie, czy plik źródłowy istnieje
    if ! check_file "$src_path"; then
        ((ERROR_COUNT++))
        continue
    fi
    
    # Utworzenie katalogu docelowego
    dest_dir="$PROJECT_LIB_DIR/$(dirname "$dest_path")"
    create_dir "$dest_dir"
    
    # Kopiowanie pliku
    copy_file "$src_path" "$PROJECT_LIB_DIR/$dest_path"
done

# Podsumowanie
if [[ $ERROR_COUNT -eq 0 ]]; then
    echo "✅ GOTOWE: Wszystkie biblioteki zostały skopiowane do $PROJECT_LIB_DIR"
else
    echo "⚠️ Ukończono z $ERROR_COUNT błędami. Sprawdź logi powyżej."
fi

# Weryfikacja
echo "📋 Weryfikacja zawartości $PROJECT_LIB_DIR:"
ls -lR "$PROJECT_LIB_DIR"
