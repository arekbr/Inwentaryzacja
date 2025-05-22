#!/bin/bash

set -e

APP_NAME="Inwentaryzacja"
BUILD_DIR="build_inwentaryzacja"

# ============================
# Wczytanie QT_PATH i QT_SRC_PATH z pliku, jeśli nie są ustawione
# ============================
if [[ (-z "$QT_PATH" || -z "$QT_SRC_PATH") && -f qt_env.sh ]]; then
    echo "ℹ️  Wczytywanie zmiennych środowiskowych z qt_env.sh"
    source qt_env.sh
fi

if [[ -z "$QT_PATH" ]]; then
    echo "❌ QT_PATH nie jest ustawiony. Użyj najpierw './1.bootstrap_linux.sh' lub ustaw ręcznie."
    exit 1
fi

# ============================
# Tworzenie katalogu build
# ============================
echo "📁 Tworzenie katalogu: $BUILD_DIR"
mkdir -p "$BUILD_DIR"

# ============================
# Konfiguracja CMake z poprawną ścieżką źródeł (tu: bieżący katalog)
# ============================
echo "⚙️ Konfiguracja CMake (Qt PATH: $QT_PATH)"
cmake -G Ninja -S . -B "$BUILD_DIR" -DCMAKE_PREFIX_PATH="$QT_PATH"

# ============================
# Budowa z użyciem Ninja
# ============================
echo "🔨 Budowanie z użyciem Ninja..."
cmake --build "$BUILD_DIR" -- -j$(nproc)

# ============================
# Weryfikacja wyniku
# ============================
echo "🔍 Sprawdzanie, czy plik binarny został utworzony..."
if [[ -f "$BUILD_DIR/$APP_NAME" ]]; then
    echo "✅ Zbudowano aplikację: $BUILD_DIR/$APP_NAME"
else
    echo "❌ Budowa nie powiodła się — brak pliku $APP_NAME"
    exit 1
fi
