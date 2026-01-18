#!/bin/bash

set -e

echo -e "\n📦 [BOOTSTRAP] Inicjalizacja środowiska pod Qt 6.9.0 + MariaDB\n"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# ==========================================
# Krok 1: Sprawdzenie systemu
# ==========================================
if [[ -f /etc/os-release ]]; then
  . /etc/os-release
  echo "ℹ️  Wykryto system: ${NAME:-unknown} ${VERSION_ID:-unknown}"
else
  echo "⚠️  Brak /etc/os-release — pomijam identyfikacje systemu."
fi

# ==========================================
# Krok 2: Instalacja wymaganych pakietów
# ==========================================
echo -e "\n🔧 Instaluję pakiety systemowe i deweloperskie..."

if ! command -v apt-get >/dev/null 2>&1; then
  echo "⚠️  Brak apt-get. Zainstaluj zaleznosci recznie i kontynuuj kolejne kroki."
else
  if sudo -n true 2>/dev/null; then
    sudo apt-get update
    sudo apt-get install -y \
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
  else
    echo "⚠️  Brak uprawnien sudo bez hasla. Pomijam instalacje pakietow."
    echo "   Uruchom recznie: sudo apt-get update && sudo apt-get install ... "
  fi
fi

# ==========================================
# Krok 3: Wskazanie ścieżki do Qt
# ==========================================
DEFAULT_QT_PATH="$HOME/Qt/6.9.0/gcc_64"
QT_PATH_DETECTED=""
if command -v qmake >/dev/null 2>&1; then
  QT_PATH_DETECTED="$(qmake -query QT_INSTALL_PREFIX 2>/dev/null || true)"
fi
if [[ -z "$QT_PATH_DETECTED" && -n "$(command -v qtpaths6)" ]]; then
  QT_PATH_DETECTED="$(qtpaths6 --install-prefix 2>/dev/null || true)"
fi
if [[ "$QT_PATH_DETECTED" == "/usr" && -d /usr/lib/qt6 ]]; then
  QT_PATH_DETECTED="/usr/lib/qt6"
fi
if [[ -z "$QT_PATH_DETECTED" && -d /usr/lib/qt6 ]]; then
  QT_PATH_DETECTED="/usr/lib/qt6"
fi

echo
if [[ -n "$QT_PATH_DETECTED" ]]; then
  read -p "📂 Wykryto Qt w: $QT_PATH_DETECTED. Użyć tej ścieżki? (Y/n) " choice
  choice="${choice:-y}"
  if [[ "$choice" == "y" || "$choice" == "Y" ]]; then
    QT_PATH="$QT_PATH_DETECTED"
  else
    read -e -p "🔍 Podaj ścieżkę do katalogu Qt [${DEFAULT_QT_PATH}]: " QT_PATH
    QT_PATH="${QT_PATH:-$DEFAULT_QT_PATH}"
  fi
else
  read -p "📂 Czy chcesz użyć własnej instalacji Qt 6.9.0? (y/n) " choice
  if [[ "$choice" == "y" ]]; then
    read -e -p "🔍 Podaj ścieżkę do katalogu Qt [${DEFAULT_QT_PATH}]: " QT_PATH
    QT_PATH="${QT_PATH:-$DEFAULT_QT_PATH}"
  else
    QT_PATH=""
  fi
fi

if [[ -n "$QT_PATH" ]]; then
  if [[ ! -d "$QT_PATH" ]]; then
    echo "❌ Błąd: Podana ścieżka nie istnieje."
    exit 1
  fi
  echo "✅ Qt PATH ustawiony na: $QT_PATH"
  export QT_PATH
  export PATH="$QT_PATH/bin:$PATH"
  export CMAKE_PREFIX_PATH="$QT_PATH"
  export QMAKE="$QT_PATH/bin/qmake"

  # Automatyczne wyznaczenie ścieżki do źródeł
  QT_SRC_PATH=""
  for candidate in \
    "$QT_PATH/../Src" \
    "$QT_PATH/../../Src" \
    "$QT_PATH/Src" \
    "$QT_PATH/qtbase"; do
    if [[ -d "$candidate/qtbase/src/plugins/sqldrivers/mysql" ]]; then
      QT_SRC_PATH="$candidate"
      break
    fi
  done

  QT_PLUGIN_DIR=""
  if command -v qtpaths6 >/dev/null 2>&1; then
    QT_PLUGIN_DIR="$(qtpaths6 --plugin-dir)"
  elif command -v qtpaths >/dev/null 2>&1; then
    QT_PLUGIN_DIR="$(qtpaths --plugin-dir)"
  fi

  if [[ -n "$QT_SRC_PATH" ]]; then
    echo "✅ Wykryto źródła Qt: $QT_SRC_PATH"
    export QT_SRC_PATH
  else
    if [[ -n "$QT_PLUGIN_DIR" && -f "$QT_PLUGIN_DIR/sqldrivers/libqsqlmysql.so" ]]; then
      echo "ℹ️  Wykryto libqsqlmysql.so w systemie. Pomijam budowę pluginu."
    else
      echo "⚠️  Źródła Qt nie zostały odnalezione. Nie mogę zbudować pluginu z kodu źródłowego."
      echo "   Zainstaluj pakiet: libqt6sql6-mysql lub dostarcz źródła Qt i ustaw QT_SRC_PATH."
    fi
    QT_SRC_PATH=""
  fi
else
  QT_PATH=""
  echo -e "\n📥 Zainstaluj Qt 6.9.0 i ustaw QT_PATH, a następnie uruchom kolejne kroki."
fi

# ==========================================
# Krok 4: Przygotowanie katalogu build/
# ==========================================
mkdir -p "$ROOT_DIR/build"
echo "✅ Utworzono katalog build/ w $ROOT_DIR"

# ==========================================
# Krok 5: Informacja końcowa
# ==========================================
if [[ -n "$QT_PATH" && -n "$QT_SRC_PATH" ]]; then
  echo -e "\n✅ Środowisko gotowe do budowy pluginu QMYSQL i projektu Inwentaryzacja"
  echo "➡️  Kolejny krok: ./2.build_qt_mysql_plugin.sh"
elif [[ -n "$QT_PATH" ]]; then
  echo -e "\n✅ Środowisko gotowe do budowy projektu Inwentaryzacja"
  echo "➡️  Kolejny krok: ./3.build_inwentaryzacja_linux.sh"
else
  echo -e "\n🛑 Zatrzymujemy się tutaj — zainstaluj Qt i ustaw QT_PATH"
fi

echo "export QT_PATH=\"$QT_PATH\"" > "$ROOT_DIR/qt_env.sh"
echo "export QT_SRC_PATH=\"$QT_SRC_PATH\"" >> "$ROOT_DIR/qt_env.sh"
