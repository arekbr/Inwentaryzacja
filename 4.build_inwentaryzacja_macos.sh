#!/bin/bash
set -e

APP_NAME="Inwentaryzacja"
BUILD_DIR="build_inwentaryzacja"

# ============================
# Wczytanie QT_PATH z pliku, jeśli nie jest ustawiony
# ============================
if [[ -z "$QT_PATH" && -f qt_env.sh ]]; then
    echo "ℹ️  Wczytywanie QT_PATH z qt_env.sh"
    source qt_env.sh
fi

if [[ -z "$QT_PATH" ]]; then
    echo "❌ QT_PATH nie jest ustawiony. Użyj najpierw 'source ./bootstrap_macos.sh' lub ustaw ręcznie."
    exit 1
fi

# ============================
# Tworzenie katalogu build
# ============================
echo "📁 Tworzenie katalogu: $BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# ============================
# Konfiguracja CMake
# ============================
echo "⚙️ Konfiguracja CMake (Qt PATH: $QT_PATH)"
cmake -G Ninja .. \
  -DCMAKE_PREFIX_PATH="$QT_PATH" \
  -DQt6Sql_NO_PLUGIN_INCLUDES=TRUE

# ============================
# Budowa Ninja
# ============================
echo "🔨 Budowanie z użyciem Ninja..."
ninja -j$(sysctl -n hw.logicalcpu)

# ============================
# Weryfikacja wyniku
# ============================
if [[ -f "$APP_NAME" ]]; then
    echo "✅ Zbudowano aplikację: $BUILD_DIR/$APP_NAME"
else
    echo "❌ Budowa nie powiodła się — brak pliku $APP_NAME"
    exit 1
fi
