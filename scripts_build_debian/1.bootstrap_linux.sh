#!/bin/bash

set -e

echo -e "\n📦 [BOOTSTRAP] Inicjalizacja środowiska pod Qt 6.9.0 + MariaDB\n"

# ==========================================
# Krok 1: Sprawdzenie systemu
# ==========================================
if ! grep -q "VERSION=\"12" /etc/os-release; then
  echo "❌ Ten skrypt wymaga systemu Debian 12 (lub zgodnego)."
  return 1
fi

echo "✅ Wykryto Debian 12"

# ==========================================
# Krok 2: Instalacja wymaganych pakietów
# ==========================================
echo -e "\n🔧 Instaluję pakiety systemowe i deweloperskie..."

sudo apt update
sudo apt install -y \
  build-essential git cmake perl python3 pkg-config \
  ninja-build curl unzip bison gperf \
  libgl1-mesa-dev libx11-dev libxext-dev libxi-dev libxrender-dev \
  libxcb1-dev libxcb-glx0-dev libxcb-keysyms1-dev libxcb-image0-dev \
  libxcb-shm0-dev libxcb-icccm4-dev libxcb-sync-dev \
  libxcb-xfixes0-dev libxcb-shape0-dev libxcb-randr0-dev \
  libxcb-render-util0-dev libxcb-xinerama0-dev \
  libxkbcommon-dev libxkbcommon-x11-dev \
  libfontconfig1-dev libfreetype6-dev libjpeg-dev libpng-dev \
  zlib1g-dev libssl-dev libicu-dev \
  libsqlite3-dev libmariadb-dev-compat libmariadb-dev

echo "✅ Pakiety zainstalowane"

# ==========================================
# Krok 3: Wskazanie ścieżki do Qt
# ==========================================
echo
read -p "📂 Czy chcesz użyć własnej instalacji Qt 6.9.0? (y/n) " choice
if [[ "$choice" == "y" ]]; then
  read -p "🔍 Podaj ścieżkę do katalogu Qt (np. /home/arekbr/Qt/6.9.0/gcc_64): " QT_PATH
  if [[ ! -d "$QT_PATH" ]]; then
    echo "❌ Błąd: Podana ścieżka nie istnieje."
    return 1
  fi
  echo "✅ Qt PATH ustawiony na: $QT_PATH"
  export QT_PATH
  export PATH="$QT_PATH/bin:$PATH"
  export CMAKE_PREFIX_PATH="$QT_PATH"
  export QMAKE="$QT_PATH/bin/qmake"

  # Automatyczne wyznaczenie ścieżki do źródeł
  QT_SRC_PATH="$(dirname "$QT_PATH")/Src"
  if [[ -d "$QT_SRC_PATH/qtbase/src/plugins/sqldrivers/mysql" ]]; then
    echo "✅ Wykryto źródła Qt: $QT_SRC_PATH"
    export QT_SRC_PATH
  else
    echo "⚠️  Źródła Qt nie zostały odnalezione w: $QT_SRC_PATH"
    echo "    Upewnij się, że Qt zostało zainstalowane z komponentem 'Src'"
    echo "    (np. przez MaintenanceTool lub ręczne pobranie z qt.io)"
  fi
else
  QT_PATH=""
  echo -e "\n📥 Qt 6.9.0 zostanie pobrane i zbudowane lokalnie w kolejnym kroku (build_qt.sh)"
fi

# ==========================================
# Krok 4: Przygotowanie katalogu build/
# ==========================================
mkdir -p build
echo "✅ Utworzono katalog build/"

# ==========================================
# Krok 5: Informacja końcowa
# ==========================================
if [[ -n "$QT_PATH" && -n "$QT_SRC_PATH" ]]; then
  echo -e "\n✅ Środowisko gotowe do budowy pluginu QMYSQL i projektu Inwentaryzacja"
  echo "➡️  Kolejny krok: ./build_qt_mysql_plugin.sh"
else
  echo -e "\n🛑 Zatrzymujemy się tutaj — kolejny krok to build_qt.sh (pełna kompilacja Qt ze źródeł)"
fi

echo "export QT_PATH=\"$QT_PATH\"" > qt_env.sh
echo "export QT_SRC_PATH=\"$QT_SRC_PATH\"" >> qt_env.sh

