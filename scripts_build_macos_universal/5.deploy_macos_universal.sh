#!/bin/bash
set -e

APP_NAME="Inwentaryzacja"
DEPLOY_DIR="deploy"

echo -e "\n📦 [DEPLOY UNIVERSAL] Deployment aplikacji (macOS Universal)\n"

# ==========================
# Wczytanie konfiguracji
# ==========================
if [[ ! -f qt_env_universal.sh ]]; then
  echo "❌ Brak pliku qt_env_universal.sh. Uruchom najpierw bootstrap."
  exit 1
fi

source qt_env_universal.sh

# ==========================
# Określenie ścieżek do aplikacji
# ==========================
case "$BUILD_TYPE" in
  "universal")
    if [[ -f "build_inwentaryzacja_universal/$APP_NAME" ]]; then
      APP_BINARY="build_inwentaryzacja_universal/$APP_NAME"
      PLUGIN_BUILD_DIRS=("build_qt_sql_drivers_universal")
      
      if [[ "$QT_DUAL_ARCH" == "true" ]]; then
        # Dla dual-arch setup, użyj ARM Qt jako głównego (nowsze)
        PRIMARY_QT_PATH="$QT_PATH_ARM"
        SECONDARY_QT_PATH="$QT_PATH_INTEL"
      else
        PRIMARY_QT_PATH="$QT_PATH"
      fi
    else
      echo "❌ Nie znaleziono universal binary aplikacji"
      exit 1
    fi
    ;;
  "native"|"intel"|"arm64")
    if [[ -f "build_inwentaryzacja_${BUILD_TYPE}/$APP_NAME" ]]; then
      APP_BINARY="build_inwentaryzacja_${BUILD_TYPE}/$APP_NAME"
      PLUGIN_BUILD_DIRS=("build_qt_sql_drivers_${BUILD_TYPE}")
      PRIMARY_QT_PATH="$QT_PATH"
    else
      echo "❌ Nie znaleziono aplikacji dla $BUILD_TYPE"
      exit 1
    fi
    ;;
  *)
    echo "❌ Nieznany BUILD_TYPE: $BUILD_TYPE"
    exit 1
    ;;
esac

echo "🎯 Aplikacja do deployment: $APP_BINARY"
echo "🎯 Główny Qt PATH: $PRIMARY_QT_PATH"

# ==========================
# Funkcja kopiowania bibliotek z filtrami architektury
# ==========================
copy_lib_universal() {
  local src_lib=$1
  local dest_dir=$2
  local target_arch=$3  # Opcjonalne: x86_64, arm64, lub universal
  
  if [[ ! -f "$src_lib" ]]; then
    echo "⚠️  Pominięto brakującą bibliotekę: $src_lib"
    return 0
  fi
  
  local lib_name=$(basename "$src_lib")
  local dest_path="$dest_dir/$lib_name"
  
  # Sprawdź architekturę biblioteki
  local lib_archs=$(lipo -archs "$src_lib" 2>/dev/null || echo "unknown")
  
  if [[ -n "$target_arch" && "$target_arch" != "universal" ]]; then
    # Filtruj do konkretnej architektury
    if echo "$lib_archs" | grep -q "$target_arch"; then
      if echo "$lib_archs" | grep -q " "; then
        # Multi-arch - wyciągnij tylko potrzebną
        echo "📦 $lib_name ($lib_archs → $target_arch)"
        lipo -thin "$target_arch" "$src_lib" -output "$dest_path"
      else
        # Single-arch
        echo "📦 $lib_name ($lib_archs)"
        cp "$src_lib" "$dest_path"
      fi
    else
      echo "⚠️  $lib_name nie ma architektury $target_arch (ma: $lib_archs)"
      return 0
    fi
  else
    # Kopiuj całość (dla universal builds)
    echo "📦 $lib_name ($lib_archs)"
    cp "$src_lib" "$dest_path"
  fi
}

# ==========================
# Czyszczenie i tworzenie katalogu deploy
# ==========================
echo "🧹 Przygotowywanie katalogu $DEPLOY_DIR..."
rm -rf "$DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR"
mkdir -p "$DEPLOY_DIR/platforms"
mkdir -p "$DEPLOY_DIR/sqldrivers"
mkdir -p "$DEPLOY_DIR/imageformats"

# ==========================
# Kopiowanie aplikacji
# ==========================
echo "📱 Kopiowanie aplikacji..."
cp "$APP_BINARY" "$DEPLOY_DIR/"

# Sprawdź architekturę skopiowanej aplikacji
APP_ARCHS=$(lipo -archs "$DEPLOY_DIR/$APP_NAME" 2>/dev/null || echo "unknown")
echo "✅ Aplikacja skopiowana (architektury: $APP_ARCHS)"

# ==========================
# Kopiowanie pluginów Qt
# ==========================
QT_PLUGIN_DIR="$PRIMARY_QT_PATH/plugins"

echo "🔌 Kopiowanie pluginów Qt z $QT_PLUGIN_DIR..."

# Platform plugins
copy_lib_universal "$QT_PLUGIN_DIR/platforms/libqcocoa.dylib" "$DEPLOY_DIR/platforms"
copy_lib_universal "$QT_PLUGIN_DIR/platforms/libqoffscreen.dylib" "$DEPLOY_DIR/platforms"

# SQL drivers (Qt standardowe)
copy_lib_universal "$QT_PLUGIN_DIR/sqldrivers/libqsqlite.dylib" "$DEPLOY_DIR/sqldrivers"

# Image formats
copy_lib_universal "$QT_PLUGIN_DIR/imageformats/libqgif.dylib" "$DEPLOY_DIR/imageformats"
copy_lib_universal "$QT_PLUGIN_DIR/imageformats/libqjpeg.dylib" "$DEPLOY_DIR/imageformats"
copy_lib_universal "$QT_PLUGIN_DIR/imageformats/libqpng.dylib" "$DEPLOY_DIR/imageformats"
copy_lib_universal "$QT_PLUGIN_DIR/imageformats/libqsvg.dylib" "$DEPLOY_DIR/imageformats"

# ==========================
# Kopiowanie zbudowanego pluginu QMYSQL
# ==========================
echo "🔍 Szukam zbudowanego pluginu QMYSQL..."

MYSQL_PLUGIN_FOUND=false
for plugin_dir in "${PLUGIN_BUILD_DIRS[@]}"; do
  MYSQL_PLUGIN_PATH="$plugin_dir/plugins/sqldrivers/libqsqlmysql.dylib"
  if [[ -f "$MYSQL_PLUGIN_PATH" ]]; then
    echo "✅ Znaleziono plugin QMYSQL: $MYSQL_PLUGIN_PATH"
    copy_lib_universal "$MYSQL_PLUGIN_PATH" "$DEPLOY_DIR/sqldrivers"
    MYSQL_PLUGIN_FOUND=true
    break
  fi
done

if [[ "$MYSQL_PLUGIN_FOUND" == false ]]; then
  echo "⚠️  Nie znaleziono pluginu QMYSQL - MySQL nie będzie dostępny"
fi

# ==========================
# Analiza i kopiowanie zależności aplikacji
# ==========================
echo -e "\n📋 Analizowanie zależności aplikacji..."

otool -L "$DEPLOY_DIR/$APP_NAME" | awk 'NR>1 {print $1}' | while read -r lib; do
  # Sprawdź czy to zależność zewnętrzna (Homebrew, Qt, itp.)
  if [[ "$lib" =~ ^(/usr/local/|/opt/homebrew/|$PRIMARY_QT_PATH) ]]; then
    echo "📥 Kopiowanie zależności: $(basename "$lib")"
    copy_lib_universal "$lib" "$DEPLOY_DIR"
  elif [[ "$lib" =~ ^@rpath/ ]]; then
    # @rpath - spróbuj znaleźć w Qt lib
    lib_name=$(basename "$lib")
    qt_lib="$PRIMARY_QT_PATH/lib/$lib_name"
    if [[ -f "$qt_lib" ]]; then
      echo "📥 Kopiowanie Qt lib: $lib_name"
      copy_lib_universal "$qt_lib" "$DEPLOY_DIR"
    fi
  fi
done

# ==========================
# Analiza zależności pluginów
# ==========================
echo -e "\n🔍 Analizowanie zależności pluginów..."

for plugin_file in "$DEPLOY_DIR/sqldrivers"/*.dylib "$DEPLOY_DIR/platforms"/*.dylib; do
  if [[ -f "$plugin_file" ]]; then
    plugin_name=$(basename "$plugin_file")
    echo "🔍 Plugin: $plugin_name"
    
    otool -L "$plugin_file" | awk 'NR>1 {print $1}' | while read -r lib; do
      if [[ "$lib" =~ ^(/usr/local/|/opt/homebrew/) ]]; then
        lib_name=$(basename "$lib")
        if [[ ! -f "$DEPLOY_DIR/$lib_name" ]]; then
          echo "📥 Kopiowanie zależności pluginu: $lib_name"
          copy_lib_universal "$lib" "$DEPLOY_DIR"
        fi
      elif [[ "$lib" =~ ^@rpath/ ]]; then
        lib_name=$(basename "$lib")
        qt_lib="$PRIMARY_QT_PATH/lib/$lib_name"
        if [[ -f "$qt_lib" && ! -f "$DEPLOY_DIR/$lib_name" ]]; then
          echo "📥 Kopiowanie Qt lib dla pluginu: $lib_name"
          copy_lib_universal "$qt_lib" "$DEPLOY_DIR"
        fi
      fi
    done
  fi
done

# ==========================
# Fixup install_name dla relocatable app
# ==========================
echo -e "\n🔧 Naprawianie ścieżek bibliotek..."

# Funkcja do naprawy install_name
fix_install_names() {
  local target_file=$1
  local is_executable=${2:-false}
  
  if [[ ! -f "$target_file" ]]; then
    return
  fi
  
  echo "🔧 Naprawianie: $(basename "$target_file")"
  
  # Napraw ścieżki do bibliotek w deploy/
  otool -L "$target_file" | awk 'NR>1 {print $1}' | while read -r lib; do
    lib_name=$(basename "$lib")
    
    # Jeśli biblioteka jest w katalogu deploy/
    if [[ -f "$DEPLOY_DIR/$lib_name" ]]; then
      if [[ "$is_executable" == true ]]; then
        # Dla aplikacji: @executable_path/
        install_name_tool -change "$lib" "@executable_path/$lib_name" "$target_file" 2>/dev/null || true
      else
        # Dla bibliotek: @loader_path/
        install_name_tool -change "$lib" "@loader_path/../$lib_name" "$target_file" 2>/dev/null || true
      fi
    fi
  done
}

# Napraw główną aplikację
fix_install_names "$DEPLOY_DIR/$APP_NAME" true

# Napraw wszystkie biblioteki
for lib_file in "$DEPLOY_DIR"/*.dylib; do
  if [[ -f "$lib_file" ]]; then
    fix_install_names "$lib_file" false
  fi
done

# Napraw pluginy
for plugin_file in "$DEPLOY_DIR"/*/*.dylib; do
  if [[ -f "$plugin_file" ]]; then
    fix_install_names "$plugin_file" false
  fi
done

# ==========================
# Utworzenie skryptu uruchamiającego
# ==========================
echo "📝 Tworzenie skryptu uruchamiającego..."

cat > "$DEPLOY_DIR/run_$APP_NAME.sh" <<EOF
#!/bin/bash
# Automatycznie wygenerowany skrypt uruchamiający

cd "\$(dirname "\$0")"

export QT_QPA_PLATFORM_PLUGIN_PATH=./platforms
export QT_PLUGIN_PATH=./sqldrivers:./imageformats
export DYLD_LIBRARY_PATH=.

echo "🚀 Uruchamianie $APP_NAME..."
echo "📁 Katalog: \$(pwd)"
echo "🏗️  Architektura aplikacji: \$(lipo -archs ./$APP_NAME 2>/dev/null || echo 'unknown')"

./$APP_NAME "\$@"
EOF

chmod +x "$DEPLOY_DIR/run_$APP_NAME.sh"

# ==========================
# Weryfikacja deployment
# ==========================
echo -e "\n✅ Deployment zakończony!"
echo "📁 Katalog: $DEPLOY_DIR/"
echo "📊 Zawartość:"

echo "   📱 Aplikacja: $APP_NAME ($(lipo -archs "$DEPLOY_DIR/$APP_NAME" 2>/dev/null || echo 'unknown'))"
echo "   🔌 Pluginy platform: $(ls "$DEPLOY_DIR/platforms/" | wc -l | tr -d ' ') plików"
echo "   🔌 Pluginy SQL: $(ls "$DEPLOY_DIR/sqldrivers/" | wc -l | tr -d ' ') plików"
echo "   📚 Biblioteki: $(ls "$DEPLOY_DIR"/*.dylib 2>/dev/null | wc -l | tr -d ' ') plików"

# Sprawdzenie MySQL
if [[ -f "$DEPLOY_DIR/sqldrivers/libqsqlmysql.dylib" ]]; then
  MYSQL_ARCHS=$(lipo -archs "$DEPLOY_DIR/sqldrivers/libqsqlmysql.dylib" 2>/dev/null || echo 'unknown')
  echo "   ✅ MySQL plugin: dostępny ($MYSQL_ARCHS)"
else
  echo "   ⚠️  MySQL plugin: niedostępny"
fi

echo -e "\n💡 Uruchamianie:"
echo "   cd $DEPLOY_DIR && ./run_$APP_NAME.sh"

echo -e "\n➡️  Kolejny krok: ./6.run_inwentaryzacja_macos_universal.sh"
