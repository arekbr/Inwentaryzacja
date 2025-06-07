#!/bin/bash
set -e

echo -e "\n🔌 [PLUGIN UNIVERSAL] Budowa Qt SQL Plugin QMYSQL - Universal macOS Build\n"

# ==========================
# Wczytanie konfiguracji
# ==========================
if [[ ! -f qt_env_universal.sh ]]; then
  echo "❌ Brak pliku qt_env_universal.sh. Uruchom najpierw ./1.bootstrap_macos_universal.sh"
  exit 1
fi

source qt_env_universal.sh

if [[ -z "$QT_SRC_PATH" ]]; then
  echo "❌ QT_SRC_PATH nie jest ustawiony. Plugin QMYSQL nie może być zbudowany."
  exit 1
fi

# ==========================
# Sprawdzenie źródeł
# ==========================
SQLDRIVERS_SRC="$QT_SRC_PATH/qtbase/src/plugins/sqldrivers"
if [[ ! -d "$SQLDRIVERS_SRC/mysql" ]]; then
  echo "❌ Brakuje źródeł mysql drivera w: $SQLDRIVERS_SRC/mysql"
  exit 1
fi

echo "✅ Źródła Qt SQL drivers: $SQLDRIVERS_SRC"

# ==========================
# Funkcja budowy dla jednej architektury
# ==========================
build_for_arch() {
  local arch=$1
  local qt_path=$2
  local mariadb_prefix=$3
  local build_suffix=$4
  
  echo -e "\n🏗️  Budowanie dla architektury: $arch"
  echo "   Qt: $qt_path"
  echo "   MariaDB prefix: $mariadb_prefix"
  
  BUILD_DIR="build_qt_sql_drivers_${build_suffix}"
  rm -rf "$BUILD_DIR"
  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR"
  
  # Inteligentne wykrywanie MariaDB
  MARIADB_LIB=""
  MARIADB_INCLUDE=""
  
  # Sprawdź różne możliwe lokalizacje MariaDB
  MARIADB_CANDIDATES=(
    "$mariadb_prefix/lib/mariadb"
    "$mariadb_prefix/lib"
    "$mariadb_prefix/opt/mariadb/lib/mariadb"
    "$mariadb_prefix/opt/mariadb/lib"
  )
  
  for candidate in "${MARIADB_CANDIDATES[@]}"; do
    if [[ -f "$candidate/libmariadb.dylib" ]]; then
      MARIADB_LIB="$candidate"
      echo "✅ Znaleziono libmariadb.dylib w: $candidate"
      break
    fi
  done
  
  if [[ -z "$MARIADB_LIB" ]]; then
    echo "❌ Nie znaleziono libmariadb.dylib. Sprawdzono:"
    for candidate in "${MARIADB_CANDIDATES[@]}"; do
      echo "   - $candidate"
    done
    return 1
  fi
  
  # Znajdź nagłówki MariaDB
  MARIADB_INCLUDE_CANDIDATES=(
    "$mariadb_prefix/include/mariadb"
    "$mariadb_prefix/include/mysql"
    "$mariadb_prefix/opt/mariadb/include/mariadb"
    "$mariadb_prefix/opt/mariadb/include/mysql"
  )
  
  for candidate in "${MARIADB_INCLUDE_CANDIDATES[@]}"; do
    if [[ -f "$candidate/mysql.h" ]]; then
      MARIADB_INCLUDE="$candidate"
      echo "✅ Znaleziono nagłówki MariaDB w: $candidate"
      break
    fi
  done
  
  if [[ -z "$MARIADB_INCLUDE" ]]; then
    echo "❌ Nie znaleziono nagłówków MariaDB (mysql.h). Sprawdzono:"
    for candidate in "${MARIADB_INCLUDE_CANDIDATES[@]}"; do
      echo "   - $candidate"
    done
    return 1
  fi
  
  # Tworzenie CMakeLists.txt
  cat > CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.20)
project(BuildQMYSQL_${arch} LANGUAGES CXX)

set(CMAKE_OSX_ARCHITECTURES "${arch}")
set(CMAKE_PREFIX_PATH "${qt_path}")
set(QT_FEATURE_sql_mysql ON)

# MariaDB target z wykrytymi ścieżkami
add_library(MySQL::MySQL UNKNOWN IMPORTED)
set_target_properties(MySQL::MySQL PROPERTIES
    IMPORTED_LOCATION "${MARIADB_LIB}/libmariadb.dylib"
    INTERFACE_INCLUDE_DIRECTORIES "${MARIADB_INCLUDE}"
)

# Dodaj kompilator flags dla odpowiedniej architektury
if("${arch}" STREQUAL "x86_64")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
else()
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
endif()

add_subdirectory("${SQLDRIVERS_SRC}" sqldrivers)
EOF

  # Budowa
  echo "🔨 Budowanie z CMake..."
  cmake -G Ninja . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="$arch" \
    -DCMAKE_PREFIX_PATH="$qt_path" \
    -DMySQL_INCLUDE_DIRS="$MARIADB_INCLUDE" \
    -DMySQL_LIBRARIES="$MARIADB_LIB/libmariadb.dylib"
  
  if ! ninja -j$(sysctl -n hw.logicalcpu 2>/dev/null || echo "4"); then
    echo "❌ Błąd podczas budowania dla $arch"
    return 1
  fi
  
  # Weryfikacja - sprawdź różne możliwe lokalizacje
  PLUGIN=""
  for plugin_path in \
    "./plugins/sqldrivers/libqsqlmysql.dylib" \
    "./sqldrivers/plugins/sqldrivers/libqsqlmysql.dylib" \
    "./lib/cmake/Qt6/plugins/sqldrivers/libqsqlmysql.dylib" \
    "$(find . -name "libqsqlmysql.dylib" | head -n1)"; do
    
    if [[ -f "$plugin_path" ]]; then
      PLUGIN="$plugin_path"
      break
    fi
  done
  
  if [[ -f "$PLUGIN" ]]; then
    echo "✅ Zbudowano plugin dla $arch: $PLUGIN"
    
    # Sprawdzenie architektury
    if file "$PLUGIN" | grep -q "$arch"; then
      echo "✅ Plugin ma poprawną architekturę $arch"
    else
      echo "⚠️  Plugin może mieć niepoprawną architekturę:"
      file "$PLUGIN"
    fi
    
    # Skopiuj do standardowej lokalizacji
    mkdir -p "plugins/sqldrivers"
    cp "$PLUGIN" "plugins/sqldrivers/libqsqlmysql.dylib"
    
  else
    echo "❌ Nie znaleziono libqsqlmysql.dylib dla $arch"
    echo "🔍 Zawartość katalogu build:"
    find . -name "*.dylib" -o -name "*.so" | head -10
    return 1
  fi
  
  cd ..
}

# ==========================
# Główna logika budowania
# ==========================
case "$BUILD_TYPE" in
  "universal")
    if [[ "$QT_DUAL_ARCH" == "true" ]]; then
      # Dwie osobne instalacje Qt
      echo "🔄 Universal build z osobnymi instalacjami Qt..."
      
      # Intel build
      INTEL_MARIADB="${INTEL_BREW_PREFIX:-/usr/local}"
      build_for_arch "x86_64" "$QT_PATH_INTEL" "$INTEL_MARIADB" "intel"
      
      # ARM build  
      ARM_MARIADB="${ARM_BREW_PREFIX:-/opt/homebrew}"
      build_for_arch "arm64" "$QT_PATH_ARM" "$ARM_MARIADB" "arm"
      
      # Łączenie w universal binary
      echo -e "\n🔗 Tworzenie universal binary..."
      mkdir -p build_qt_sql_drivers_universal/plugins/sqldrivers
      
      lipo -create \
        build_qt_sql_drivers_intel/plugins/sqldrivers/libqsqlmysql.dylib \
        build_qt_sql_drivers_arm/plugins/sqldrivers/libqsqlmysql.dylib \
        -output build_qt_sql_drivers_universal/plugins/sqldrivers/libqsqlmysql.dylib
      
      # Weryfikacja
      if file build_qt_sql_drivers_universal/plugins/sqldrivers/libqsqlmysql.dylib | grep -q "universal binary"; then
        echo "✅ Utworzono universal binary plugin"
        lipo -info build_qt_sql_drivers_universal/plugins/sqldrivers/libqsqlmysql.dylib
      else
        echo "❌ Nie udało się utworzyć universal binary"
        exit 1
      fi
      
    elif [[ "$QT_UNIVERSAL" == "true" ]]; then
      # Jedna instalacja Qt z universal binaries
      echo "🔄 Universal build z universal Qt..."
      
      # Inteligentne wykrywanie MariaDB dla universal build
      MARIADB_PREFIX=""
      if [[ -f "/opt/homebrew/lib/libmariadb.dylib" ]]; then
        MARIADB_PREFIX="/opt/homebrew"
        echo "✅ Używam ARM MariaDB: $MARIADB_PREFIX"
      elif [[ -f "/usr/local/lib/libmariadb.dylib" ]]; then
        MARIADB_PREFIX="/usr/local" 
        echo "✅ Używam Intel MariaDB: $MARIADB_PREFIX"
      else
        echo "❌ Nie znaleziono libmariadb.dylib w standardowych lokalizacjach"
        echo "🔍 Sprawdź czy MariaDB jest zainstalowane przez Homebrew"
        exit 1
      fi
      
      build_for_arch "x86_64;arm64" "$QT_PATH" "$MARIADB_PREFIX" "universal"
    else
      echo "❌ Universal build wymaga universal Qt lub dual-arch setup"
      exit 1
    fi
    ;;
    
  "native"|"intel"|"arm64")
    # Single architecture build
    echo "🔄 Single architecture build: $BUILD_TYPE"
    
    ARCH="$CMAKE_OSX_ARCHITECTURES"
    QT_PATH_TO_USE="$QT_PATH"
    
    # Wybór właściwego MariaDB
    if [[ "$ARCH" == "arm64" ]]; then
      MARIADB_PREFIX="/opt/homebrew"
    else
      MARIADB_PREFIX="/usr/local"
    fi
    
    # Fallback - sprawdź co jest dostępne
    if [[ ! -f "$MARIADB_PREFIX/lib/libmariadb.dylib" ]]; then
      if [[ -f "/opt/homebrew/lib/libmariadb.dylib" ]]; then
        MARIADB_PREFIX="/opt/homebrew"
      elif [[ -f "/usr/local/lib/libmariadb.dylib" ]]; then
        MARIADB_PREFIX="/usr/local"
      else
        echo "❌ Nie znaleziono libmariadb.dylib"
        exit 1
      fi
    fi
    
    build_for_arch "$ARCH" "$QT_PATH_TO_USE" "$MARIADB_PREFIX" "$BUILD_TYPE"
    ;;
    
  *)
    echo "❌ Nieznany BUILD_TYPE: $BUILD_TYPE"
    exit 1
    ;;
esac

echo -e "\n✅ Plugin QMYSQL zbudowany pomyślnie!"
echo "➡️  Kolejny krok: ./3.sanitize_qt_sql_plugins_macos_universal.sh (opcjonalny)"
echo "➡️  Lub przejdź do: ./4.build_inwentaryzacja_macos_universal.sh"
