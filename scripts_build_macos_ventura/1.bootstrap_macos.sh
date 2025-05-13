#!/bin/bash
set -e
export CMAKE_OSX_ARCHITECTURES=$(uname -m)

echo -e "\n🍏 [BOOTSTRAP] Inicjalizacja środowiska pod Qt 6.9.0 + MariaDB (macOS Ventura+)\n"

# ==========================================
# Krok 1: Sprawdzenie systemu
# ==========================================
OS_VERSION=$(sw_vers -productVersion)
if [[ "$OS_VERSION" < "13.0" ]]; then
  echo "❌ Ten skrypt wymaga macOS Ventura (13.0) lub nowszego."
  exit 1
fi

echo "✅ Wykryto macOS $OS_VERSION"

# ==========================================
# Krok 2: Sprawdzenie Xcode i SDK
# ==========================================
if ! xcode-select -p &>/dev/null; then
  echo "❌ Xcode Command Line Tools nie są zainstalowane."
  echo "🔧 Uruchom: xcode-select --install"
  exit 1
fi

XCODE_VERSION=$(xcodebuild -version | grep Xcode | awk '{print $2}')
SDK_VERSION=$(xcrun --sdk macosx --show-sdk-version)

echo "🔍 Wykryto Xcode $XCODE_VERSION, SDK $SDK_VERSION"

# ==========================================
# Bonus: pełna lista dostępnych SDK i decyzja usera
# ==========================================
echo -e "\n🔎 Sprawdzam dostępne SDK:"
AVAILABLE_SDKS=$(xcodebuild -showsdks | grep macosx | awk '{print $NF}' | sed 's/macosx//g' | sort -V)
echo "➡️  Znalezione SDK macOS: $AVAILABLE_SDKS"

SDK_MIN="14.0"
SDK_OK=false

for ver in $AVAILABLE_SDKS; do
  if [[ "$(printf '%s\n' "$ver" "$SDK_MIN" | sort -V | head -n1)" == "$SDK_MIN" ]]; then
    SDK_OK=true
    break
  fi
done

if [[ "$SDK_OK" == false ]]; then
  echo -e "\n⚠️  Qt 6.9.0 oficjalnie wymaga SDK >= 14.0 (w szczególności przy budowie Qt ze źródeł)"
  echo "Twoja aktualna wersja SDK to: $SDK_VERSION"
  echo
  echo "🛠️  Jeśli używasz gotowego Qt 6.9.0 (np. z MaintenanceTool), to SDK 13.x może być wystarczające"
  echo "   — pod warunkiem, że nie korzystasz z funkcji wymagających nowego API."
  echo
  echo "📥 Aby zainstalować SDK 14+, pobierz Xcode 15.2:"
  echo "🔗 https://download.developer.apple.com/Developer_Tools/Xcode_15.2/Xcode_15.2.xip"
  echo
  echo "📌 Po instalacji ustaw aktywne Xcode:"
  echo "   sudo xcode-select -s /Applications/Xcode.app"
  echo

  read -p "❓ Czy mimo to chcesz kontynuować z SDK $SDK_VERSION? (y/n) " answer
  if [[ "$answer" != "y" ]]; then
    echo "🚪 Zakończono. Zainstaluj Xcode z SDK 14+ i uruchom ponownie."
    exit 1
  else
    echo "➡️ Kontynuujemy z SDK $SDK_VERSION — na odpowiedzialność użytkownika."
  fi
else
  echo "✅ SDK >= 14.0 dostępny — OK"
fi

# ==========================================
# Krok 3: Instalacja pakietów Homebrew
# ==========================================
echo -e "\n🍺 Instaluję wymagane pakiety przez Homebrew..."

brew update
brew install cmake ninja git mariadb openssl@3 pkg-config sqlite3 icu4c

echo "✅ Pakiety zainstalowane"

# ==========================================
# Krok 4: Wybór Qt
# ==========================================
echo
read -p "📂 Czy chcesz użyć własnej instalacji Qt 6.9.0? (y/n) " choice
if [[ "$choice" == "y" ]]; then
    DEFAULT_QT_PATH="$HOME/Qt/6.9.0/macos"
    read -p "🔍 Podaj ścieżkę do katalogu Qt [$DEFAULT_QT_PATH]: " QT_PATH
    QT_PATH="${QT_PATH:-$DEFAULT_QT_PATH}"
  if [[ ! -d "$QT_PATH" ]]; then
    echo "❌ Błąd: Podana ścieżka nie istnieje."
    exit 1
  fi
  echo "✅ Qt PATH ustawiony na: $QT_PATH"
  export QT_PATH
  export PATH="$QT_PATH/bin:$PATH"
  export CMAKE_PREFIX_PATH="$QT_PATH"
  export QMAKE="$QT_PATH/bin/qmake"

  echo "🔍 Szukam źródeł Qt (qtbase/src/plugins/sqldrivers)..."
  CANDIDATES=(
    "$(dirname "$QT_PATH")/Src"
    "$(dirname "$(dirname "$QT_PATH")")/Src"
    "$QT_PATH/Src"
  )

  QT_SRC_PATH=""
  for candidate in "${CANDIDATES[@]}"; do
    test_path="$candidate/qtbase/src/plugins/sqldrivers"
    echo "🔎 Sprawdzam: $test_path"
    if [[ -d "$test_path" ]]; then
      QT_SRC_PATH="$candidate"
      echo "✅ Wykryto źródła Qt: $QT_SRC_PATH"
      break
    fi
  done

  if [[ -n "$QT_SRC_PATH" ]]; then
    export QT_SRC_PATH
  else
    echo -e "\n🛑 Nie znaleziono katalogu qtbase/src/plugins/sqldrivers w żadnej z prób:"
    for candidate in "${CANDIDATES[@]}"; do
      echo "   ✖ $candidate"
    done
    echo
    echo "📌 Upewnij się, że Qt zostało zainstalowane z komponentem 'Sources'"
    echo "🔗 Lub pobierz ręcznie z: https://download.qt.io/official_releases/qt/6.9/6.9.0/single/"
  fi
else
  QT_PATH=""
  echo -e "\n📥 Qt 6.9.0 zostanie pobrane i zbudowane lokalnie w kolejnym kroku (build_qt.sh)"
fi

# ==========================================
# Krok 5: Utworzenie katalogu build/
# ==========================================
mkdir -p build
echo "✅ Utworzono katalog build/"

# ==========================================
# Krok 6: Informacja końcowa
# ==========================================
if [[ -n "$QT_PATH" && -d "$QT_SRC_PATH/qtbase/src/plugins/sqldrivers" ]]; then

  echo -e "\n✅ Środowisko gotowe do budowy pluginu QMYSQL i projektu Inwentaryzacja"
  echo "➡️  Kolejny krok: ./2.build_qt_mysql_plugin_macos.sh"
else
  echo -e "\n🛑 Zatrzymujemy się tutaj — kolejny krok to build_qt.sh (pełna kompilacja Qt ze źródeł)"
fi

# ==========================================
# Krok 7: Zapis do qt_env.sh
# ==========================================
echo "export QT_PATH=\"$QT_PATH\"" > qt_env.sh
echo "export QT_SRC_PATH=\"$QT_SRC_PATH\"" >> qt_env.sh
echo "# (Wygenerowano automatycznie przez bootstrap_macos.sh)" >> qt_env.sh
