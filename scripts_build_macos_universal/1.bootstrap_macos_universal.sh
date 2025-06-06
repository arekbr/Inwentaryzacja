#!/bin/bash
set -e

echo -e "\n🍏 [BOOTSTRAP UNIVERSAL] Inicjalizacja środowiska dla Universal macOS Build (Intel + Apple Silicon)\n"

# ==========================================
# Krok 1: Wykrywanie architektury systemu
# ==========================================
HOST_ARCH=$(uname -m)
OS_VERSION=$(sw_vers -productVersion)

if [[ "$OS_VERSION" < "13.0" ]]; then
  echo "❌ Ten skrypt wymaga macOS Ventura (13.0) lub nowszego."
  exit 1
fi

echo "✅ Wykryto macOS $OS_VERSION na architekturze $HOST_ARCH"

# ==========================================
# Krok 2: Wybór trybu budowania
# ==========================================
echo -e "\n📋 Dostępne tryby budowania:"
echo "1) Universal Binary (x86_64 + arm64) - zalecane"
echo "2) Native ($HOST_ARCH only)"
echo "3) Intel only (x86_64)"
echo "4) Apple Silicon only (arm64)"

read -p "❓ Wybierz tryb (1-4) [1]: " BUILD_MODE
BUILD_MODE="${BUILD_MODE:-1}"

case $BUILD_MODE in
  1)
    export CMAKE_OSX_ARCHITECTURES="x86_64;arm64"
    export BUILD_TYPE="universal"
    echo "🎯 Wybrano: Universal Binary (x86_64 + arm64)"
    ;;
  2)
    export CMAKE_OSX_ARCHITECTURES="$HOST_ARCH"
    export BUILD_TYPE="native"
    echo "🎯 Wybrano: Native build ($HOST_ARCH)"
    ;;
  3)
    export CMAKE_OSX_ARCHITECTURES="x86_64"
    export BUILD_TYPE="intel"
    echo "🎯 Wybrano: Intel only (x86_64)"
    ;;
  4)
    export CMAKE_OSX_ARCHITECTURES="arm64"
    export BUILD_TYPE="arm64"
    echo "🎯 Wybrano: Apple Silicon only (arm64)"
    ;;
  *)
    echo "❌ Nieprawidłowy wybór"
    exit 1
    ;;
esac

# ==========================================
# Krok 3: Sprawdzenie Xcode i SDK
# ==========================================
if ! xcode-select -p &>/dev/null; then
  echo "❌ Xcode Command Line Tools nie są zainstalowane."
  echo "🔧 Uruchom: xcode-select --install"
  exit 1
fi

XCODE_VERSION=$(xcodebuild -version | grep Xcode | awk '{print $2}')
SDK_VERSION=$(xcrun --sdk macosx --show-sdk-version)

echo "🔍 Wykryto Xcode $XCODE_VERSION, SDK $SDK_VERSION"

# Sprawdzenie wymagań dla universal builds
if [[ "$BUILD_TYPE" == "universal" ]]; then
  XCODE_MAJOR=$(echo $XCODE_VERSION | cut -d. -f1)
  if [[ "$XCODE_MAJOR" -lt "15" ]]; then
    echo "⚠️  Universal builds zalecają Xcode 15+ dla najlepszej kompatybilności"
    read -p "❓ Kontynuować mimo to? (y/n) " answer
    if [[ "$answer" != "y" ]]; then
      exit 1
    fi
  fi
fi

# ==========================================
# Krok 4: Instalacja pakietów Homebrew
# ==========================================
echo -e "\n🍺 Instaluję wymagane pakiety przez Homebrew..."

# Dla universal builds potrzebujemy czasem obie architektury
if [[ "$BUILD_TYPE" == "universal" && "$HOST_ARCH" == "arm64" ]]; then
  echo "🔄 Dla universal build na Apple Silicon, sprawdzam dostępność Intel binaries..."
  
  # Instalacja podstawowych pakietów
  brew update
  brew install cmake ninja git mariadb openssl@3 pkg-config sqlite3 icu4c
  
  # Sprawdzenie czy mamy też Intel homebrew (Rosetta)
  if [[ -d "/usr/local/Homebrew" ]]; then
    echo "✅ Wykryto Intel Homebrew - możliwy full universal build"
    export INTEL_BREW_PREFIX="/usr/local"
    export ARM_BREW_PREFIX="/opt/homebrew"
  else
    echo "⚠️  Brak Intel Homebrew - universal build będzie oparty na ARM + cross-compile"
    export ARM_BREW_PREFIX="/opt/homebrew"
  fi
else
  brew update
  brew install cmake ninja git mariadb openssl@3 pkg-config sqlite3 icu4c
fi

echo "✅ Pakiety zainstalowane"

# ==========================================
# Krok 5: Konfiguracja Qt dla universal builds
# ==========================================
echo -e "\n📱 Konfiguracja Qt..."

read -p "📂 Czy chcesz użyć własnej instalacji Qt 6.9.0? (y/n) " choice
if [[ "$choice" == "y" ]]; then
  if [[ "$BUILD_TYPE" == "universal" ]]; then
    echo "🔍 Dla universal build potrzebujesz Qt skompilowane jako universal binary"
    echo "📌 Opcje:"
    echo "   1) Qt z MaintenanceTool (sprawdź czy ma universal support)"
    echo "   2) Własne Qt zbudowane jako universal"
    echo "   3) Osobne instalacje Qt dla każdej architektury"
    
    read -p "❓ Którą opcję wybierasz? (1/2/3) " qt_option
    
    case $qt_option in
      1|2)
        DEFAULT_QT_PATH="$HOME/Qt/6.9.0/macos"
        read -p "🔍 Podaj ścieżkę do katalogu Qt [$DEFAULT_QT_PATH]: " QT_PATH
        QT_PATH="${QT_PATH:-$DEFAULT_QT_PATH}"
        
        if [[ ! -d "$QT_PATH" ]]; then
          echo "❌ Błąd: Podana ścieżka nie istnieje."
          exit 1
        fi
        
        # Sprawdzenie czy Qt ma universal binaries
        if file "$QT_PATH/bin/qmake" | grep -q "universal binary"; then
          echo "✅ Qt ma universal binaries"
          export QT_PATH
          export QT_UNIVERSAL=true
        else
          echo "⚠️  Qt nie ma universal binaries - będzie single-arch build"
          export QT_PATH
          export QT_UNIVERSAL=false
        fi
        ;;
      3)
        read -p "🔍 Podaj ścieżkę do Qt Intel (x86_64): " QT_PATH_INTEL
        read -p "🔍 Podaj ścieżkę do Qt ARM (arm64): " QT_PATH_ARM
        
        if [[ ! -d "$QT_PATH_INTEL" || ! -d "$QT_PATH_ARM" ]]; then
          echo "❌ Błąd: Jedna z podanych ścieżek nie istnieje."
          exit 1
        fi
        
        export QT_PATH_INTEL
        export QT_PATH_ARM
        export QT_DUAL_ARCH=true
        ;;
    esac
  else
    # Single architecture build
    DEFAULT_QT_PATH="$HOME/Qt/6.9.0/macos"
    read -p "🔍 Podaj ścieżkę do katalogu Qt [$DEFAULT_QT_PATH]: " QT_PATH
    QT_PATH="${QT_PATH:-$DEFAULT_QT_PATH}"
    
    if [[ ! -d "$QT_PATH" ]]; then
      echo "❌ Błąd: Podana ścieżka nie istnieje."
      exit 1
    fi
    
    export QT_PATH
  fi
  
  # Wyszukiwanie źródeł Qt
  echo "🔍 Szukam źródeł Qt..."
  if [[ "$QT_DUAL_ARCH" == "true" ]]; then
    # Sprawdź źródła dla pierwszej architektury
    CANDIDATES=(
      "$(dirname "$QT_PATH_INTEL")/Src"
      "$(dirname "$(dirname "$QT_PATH_INTEL")")/Src"
      "$QT_PATH_INTEL/Src"
    )
  else
    CANDIDATES=(
      "$(dirname "$QT_PATH")/Src"
      "$(dirname "$(dirname "$QT_PATH")")/Src"
      "$QT_PATH/Src"
    )
  fi

  QT_SRC_PATH=""
  for candidate in "${CANDIDATES[@]}"; do
    test_path="$candidate/qtbase/src/plugins/sqldrivers"
    if [[ -d "$test_path" ]]; then
      QT_SRC_PATH="$candidate"
      echo "✅ Wykryto źródła Qt: $QT_SRC_PATH"
      break
    fi
  done

  if [[ -z "$QT_SRC_PATH" ]]; then
    echo "🛑 Nie znaleziono źródeł Qt - plugin QMYSQL będzie pomijany"
  fi
  
  export QT_SRC_PATH
else
  echo "📥 Qt zostanie pobrane i zbudowane lokalnie (nie implementowane w tym skrypcie)"
  echo "❌ Dla uproszczenia, użyj gotowej instalacji Qt"
  exit 1
fi

# ==========================================
# Krok 6: Utworzenie katalogu build
# ==========================================
mkdir -p build

# ==========================================
# Krok 7: Zapis konfiguracji
# ==========================================
echo "💾 Zapisywanie konfiguracji..."

cat > qt_env_universal.sh <<EOF
# Wygenerowano automatycznie przez bootstrap_macos_universal.sh
# $(date)

export BUILD_TYPE="$BUILD_TYPE"
export CMAKE_OSX_ARCHITECTURES="$CMAKE_OSX_ARCHITECTURES"
export HOST_ARCH="$HOST_ARCH"

EOF

if [[ "$QT_DUAL_ARCH" == "true" ]]; then
cat >> qt_env_universal.sh <<EOF
export QT_DUAL_ARCH="true"
export QT_PATH_INTEL="$QT_PATH_INTEL"
export QT_PATH_ARM="$QT_PATH_ARM"
EOF
else
cat >> qt_env_universal.sh <<EOF
export QT_PATH="$QT_PATH"
export QT_UNIVERSAL="$QT_UNIVERSAL"
EOF
fi

if [[ -n "$QT_SRC_PATH" ]]; then
  echo "export QT_SRC_PATH=\"$QT_SRC_PATH\"" >> qt_env_universal.sh
fi

if [[ -n "$INTEL_BREW_PREFIX" ]]; then
  echo "export INTEL_BREW_PREFIX=\"$INTEL_BREW_PREFIX\"" >> qt_env_universal.sh
fi

if [[ -n "$ARM_BREW_PREFIX" ]]; then
  echo "export ARM_BREW_PREFIX=\"$ARM_BREW_PREFIX\"" >> qt_env_universal.sh
fi

echo "✅ Konfiguracja zapisana do qt_env_universal.sh"

# ==========================================
# Krok 8: Informacja końcowa
# ==========================================
echo -e "\n🎉 Bootstrap zakończony!"
echo "📋 Konfiguracja:"
echo "   • Tryb budowania: $BUILD_TYPE"
echo "   • Architektura(y): $CMAKE_OSX_ARCHITECTURES"
echo "   • Źródła Qt: ${QT_SRC_PATH:-"brak"}"

if [[ -n "$QT_SRC_PATH" ]]; then
  echo -e "\n➡️  Kolejny krok: ./2.build_qt_mysql_plugin_macos_universal.sh"
else
  echo -e "\n➡️  Kolejny krok: ./4.build_inwentaryzacja_macos_universal.sh (pomijając plugin MySQL)"
fi

echo -e "\n💡 Aby załadować środowisko w nowej sesji:"
echo "   source qt_env_universal.sh"
