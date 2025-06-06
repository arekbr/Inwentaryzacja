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
  echo "   MariaDB: $mariadb_prefix"
  
  BUILD_DIR="build_qt_sql_drivers_${build_suffix}"
  rm -rf "$BUILD_DIR"
  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR"
  
  # Sprawdzenie MariaDB
  if [[ ! -f "$mariadb_prefix/lib/libmariadb.dylib" ]]; then
    echo "❌ Nie znaleziono libmariadb.dylib w $mariadb_prefix/lib/"
    return 1
  fi
  
  # Tworzenie CMakeLists.txt
  cat > CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.20)
project(BuildQMYSQL_${arch} LANGUAGES CXX)

set(CMAKE_OSX_ARCHITECTURES "${arch}")
set(CMAKE_PREFIX_PATH "${qt_path}")
set(QT_FEATURE_sql_mysql ON)

# MariaDB target
add_library(MySQL::MySQL UNKNOWN IMPORTED)
set_target_properties(MySQL::MySQL PROPERTIES
    IMPORTED_LOCATION "${mariadb_prefix}/lib/libmariadb.dylib"
    INTERFACE_INCLUDE_DIRECTORIES "${mariadb_prefix}/include/mysql"
)

add_subdirectory("${SQLDRIVERS_SRC}" sqldrivers)
EOF

  # Budowa
  cmake -G Ninja . \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES="$arch" \
    -DCMAKE_PREFIX_PATH="$qt_path"
  
  ninja -j$(sysctl -n hw.logicalcpu)
  
  # Weryfikacja
  PLUGIN=$(find . -name "libqsqlmysql.dylib" | head -n1)
  if [[ -f "$PLUGIN" ]]; then
    echo "✅ Zbudowano plugin dla $arch: $PLUGIN"
    
    # Sprawdzenie architektury
    if file "$PLUGIN" | grep -q "$arch"; then
      echo "✅ Plugin ma poprawną architekturę $arch"
    else
      echo "⚠️  Plugin może mieć niepoprawną architekturę:"
      file "$PLUGIN"
    fi
  else
    echo "❌ Nie znaleziono libqsqlmysql.dylib dla $arch"
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
      
      MARIADB_PREFIX="/opt/homebrew"  # Domyślnie ARM homebrew
      
      BUILD_DIR="build_qt_sql_drivers_universal"
      rm -rf "$BUILD_DIR"
      mkdir -p "$BUILD_DIR"
      cd "$BUILD_DIR"
      
      # Sprawdzenie MariaDB - może być potrzebne oba
      if [[ -f "/opt/homebrew/lib/libmariadb.dylib" ]]; then
        MARIADB_PREFIX="/opt/homebrew"
      elif [[ -f "/usr/local/lib/libmariadb.dylib" ]]; then
        MARIADB_PREFIX="/usr/local"
      else
        echo "❌ Nie znaleziono libmariadb.dylib"
        exit 1
      fi
      
      cat > CMakeLists.txt <<EOF
cmake_minimum_required(VERSION 3.20)
project(BuildQMYSQL_Universal LANGUAGES CXX)

set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
set(CMAKE_PREFIX_PATH "${QT_PATH}")
set(QT_FEATURE_sql_mysql ON)

add_library(MySQL::MySQL UNKNOWN IMPORTED)
set_target_properties(MySQL::MySQL PROPERTIES
    IMPORTED_LOCATION "${MARIADB_PREFIX}/lib/libmariadb.dylib"
    INTERFACE_INCLUDE_DIRECTORIES "${MARIADB_PREFIX}/include/mysql"
)

add_subdirectory("${SQLDRIVERS_SRC}" sqldrivers)
EOF

      cmake -G Ninja . \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
        -DCMAKE_PREFIX_PATH="$QT_PATH"
      
      ninja -j$(sysctl -n hw.logicalcpu)
      
      PLUGIN=$(find . -name "libqsqlmysql.dylib" | head -n1)
      if [[ -f "$PLUGIN" ]]; then
        echo "✅ Zbudowano universal plugin: $PLUGIN"
        lipo -info "$PLUGIN"
      else
        echo "❌ Nie znaleziono universal plugin"
        exit 1
      fi
      
      cd ..
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
