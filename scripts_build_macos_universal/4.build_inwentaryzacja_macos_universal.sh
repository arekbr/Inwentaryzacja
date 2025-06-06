#!/bin/bash
set -e

APP_NAME="Inwentaryzacja"

echo -e "\n🏗️  [BUILD UNIVERSAL] Budowa aplikacji Inwentaryzacja (macOS Universal)\n"

# ==========================
# Wczytanie konfiguracji
# ==========================
if [[ ! -f qt_env_universal.sh ]]; then
  echo "❌ Brak pliku qt_env_universal.sh. Uruchom najpierw bootstrap."
  exit 1
fi

source qt_env_universal.sh

# ==========================
# Funkcja budowy dla jednej architektury
# ==========================
build_for_arch() {
  local arch=$1
  local qt_path=$2
  local build_suffix=$3
  
  echo -e "\n🔨 Budowanie aplikacji dla architektury: $arch"
  echo "   Qt: $qt_path"
  
  BUILD_DIR="build_inwentaryzacja_${build_suffix}"
  
  echo "📁 Przygotowywanie katalogu: $BUILD_DIR"
  rm -rf "$BUILD_DIR"
  mkdir -p "$BUILD_DIR"
  cd "$BUILD_DIR"
  
  echo "⚙️  Konfiguracja CMake..."
  cmake -G Ninja .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$qt_path" \
    -DCMAKE_OSX_ARCHITECTURES="$arch" \
    -DQt6Sql_NO_PLUGIN_INCLUDES=TRUE
  
  echo "🔨 Budowanie z Ninja..."
  ninja -j$(sysctl -n hw.logicalcpu)
  
  # Weryfikacja
  if [[ -f "$APP_NAME" ]]; then
    echo "✅ Zbudowano aplikację dla $arch: $BUILD_DIR/$APP_NAME"
    
    # Sprawdzenie architektury
    if file "$APP_NAME" | grep -q "$arch"; then
      echo "✅ Aplikacja ma poprawną architekturę $arch"
    else
      echo "⚠️  Aplikacja może mieć niepoprawną architekturę:"
      file "$APP_NAME"
    fi
    
    # Sprawdzenie linkowania Qt
    echo "🔍 Sprawdzanie zależności Qt..."
    otool -L "$APP_NAME" | grep -E "(Qt6|@rpath)" | head -5
    
  else
    echo "❌ Budowa nie powiodła się - brak pliku $APP_NAME"
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
      build_for_arch "x86_64" "$QT_PATH_INTEL" "intel"
      
      # ARM build  
      build_for_arch "arm64" "$QT_PATH_ARM" "arm"
      
      # Łączenie w universal binary
      echo -e "\n🔗 Tworzenie universal binary aplikacji..."
      
      mkdir -p "build_inwentaryzacja_universal"
      
      lipo -create \
        "build_inwentaryzacja_intel/$APP_NAME" \
        "build_inwentaryzacja_arm/$APP_NAME" \
        -output "build_inwentaryzacja_universal/$APP_NAME"
      
      # Weryfikacja
      if file "build_inwentaryzacja_universal/$APP_NAME" | grep -q "universal binary"; then
        echo "✅ Utworzono universal binary aplikacji"
        lipo -info "build_inwentaryzacja_universal/$APP_NAME"
        
        # Sprawdzenie zależności
        echo "🔍 Sprawdzanie zależności universal binary..."
        otool -L "build_inwentaryzacja_universal/$APP_NAME" | grep -E "(Qt6|@rpath)" | head -5
        
      else
        echo "❌ Nie udało się utworzyć universal binary"
        exit 1
      fi
      
    elif [[ "$QT_UNIVERSAL" == "true" ]]; then
      # Jedna instalacja Qt z universal binaries
      echo "🔄 Universal build z universal Qt..."
      
      BUILD_DIR="build_inwentaryzacja_universal"
      rm -rf "$BUILD_DIR"
      mkdir -p "$BUILD_DIR"
      cd "$BUILD_DIR"
      
      echo "⚙️  Konfiguracja CMake dla universal build..."
      cmake -G Ninja .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH="$QT_PATH" \
        -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
        -DQt6Sql_NO_PLUGIN_INCLUDES=TRUE
      
      echo "🔨 Budowanie universal binary..."
      ninja -j$(sysctl -n hw.logicalcpu)
      
      if [[ -f "$APP_NAME" ]]; then
        echo "✅ Zbudowano universal aplikację: $BUILD_DIR/$APP_NAME"
        
        if file "$APP_NAME" | grep -q "universal binary"; then
          echo "✅ Aplikacja jest universal binary"
          lipo -info "$APP_NAME"
        else
          echo "⚠️  Aplikacja może nie być universal binary:"
          file "$APP_NAME"
        fi
        
        # Sprawdzenie zależności
        echo "🔍 Sprawdzanie zależności..."
        otool -L "$APP_NAME" | grep -E "(Qt6|@rpath)" | head -5
        
      else
        echo "❌ Budowa nie powiodła się"
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
    
    build_for_arch "$ARCH" "$QT_PATH_TO_USE" "$BUILD_TYPE"
    ;;
    
  *)
    echo "❌ Nieznany BUILD_TYPE: $BUILD_TYPE"
    exit 1
    ;;
esac

echo -e "\n🎉 Budowa aplikacji zakończona pomyślnie!"

# Informacja o następnych krokach
if [[ "$BUILD_TYPE" == "universal" ]]; then
  if [[ -f "build_inwentaryzacja_universal/$APP_NAME" ]]; then
    echo "📁 Universal binary aplikacji: build_inwentaryzacja_universal/$APP_NAME"
  fi
else
  if [[ -f "build_inwentaryzacja_${BUILD_TYPE}/$APP_NAME" ]]; then
    echo "📁 Aplikacja ($BUILD_TYPE): build_inwentaryzacja_${BUILD_TYPE}/$APP_NAME"
  fi
fi

echo -e "\n➡️  Kolejny krok: ./5.deploy_macos_universal.sh"
