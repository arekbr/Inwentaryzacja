#!/bin/bash
set -e

APP_NAME="Inwentaryzacja"
DEPLOY_DIR="deploy"
DMG_NAME="${APP_NAME}_Universal"

echo -e "\n📦 [DMG UNIVERSAL] Tworzenie pakietu DMG (macOS Universal)\n"

# ==========================
# Wczytanie konfiguracji
# ==========================
if [[ ! -f qt_env_universal.sh ]]; then
  echo "❌ Brak pliku qt_env_universal.sh. Uruchom najpierw bootstrap."
  exit 1
fi

source qt_env_universal.sh

# ==========================
# Sprawdzenie deployment
# ==========================
if [[ ! -f "$DEPLOY_DIR/$APP_NAME" ]]; then
  echo "❌ Nie znaleziono aplikacji w $DEPLOY_DIR/$APP_NAME"
  echo "🔧 Uruchom najpierw: ./5.deploy_macos_universal.sh"
  exit 1
fi

# ==========================
# Odczyt wersji aplikacji
# ==========================
if [[ -f "version.txt" ]]; then
  APP_VERSION=$(cat version.txt | tr -d '\n\r')
  echo "📋 Wersja aplikacji: $APP_VERSION"
else
  APP_VERSION="1.0.0"
  echo "⚠️  Nie znaleziono version.txt, używam domyślnej wersji: $APP_VERSION"
fi

# ==========================
# Informacje o architekturze
# ==========================
APP_ARCHS=$(lipo -archs "$DEPLOY_DIR/$APP_NAME" 2>/dev/null || echo "unknown")
echo "🏗️  Architektury w aplikacji: $APP_ARCHS"

if echo "$APP_ARCHS" | grep -q "universal\|x86_64.*arm64\|arm64.*x86_64"; then
  ARCH_SUFFIX="_Universal"
elif echo "$APP_ARCHS" | grep -q "arm64"; then
  ARCH_SUFFIX="_AppleSilicon"
elif echo "$APP_ARCHS" | grep -q "x86_64"; then
  ARCH_SUFFIX="_Intel"
else
  ARCH_SUFFIX=""
fi

DMG_FILENAME="${APP_NAME}_${APP_VERSION}${ARCH_SUFFIX}.dmg"

echo "📦 Nazwa DMG: $DMG_FILENAME"

# ==========================
# Przygotowanie struktury aplikacji macOS
# ==========================
echo "📁 Przygotowywanie bundle aplikacji..."

APP_BUNDLE="${APP_NAME}.app"
TEMP_DIR="temp_dmg"

rm -rf "$TEMP_DIR"
mkdir -p "$TEMP_DIR/$APP_BUNDLE/Contents/MacOS"
mkdir -p "$TEMP_DIR/$APP_BUNDLE/Contents/Resources"
mkdir -p "$TEMP_DIR/$APP_BUNDLE/Contents/Frameworks"
mkdir -p "$TEMP_DIR/$APP_BUNDLE/Contents/PlugIns"

# ==========================
# Tworzenie Info.plist
# ==========================
echo "📝 Tworzenie Info.plist..."

cat > "$TEMP_DIR/$APP_BUNDLE/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>$APP_NAME</string>
    <key>CFBundleIdentifier</key>
    <string>com.arekbr.inwentaryzacja</string>
    <key>CFBundleName</key>
    <string>$APP_NAME</string>
    <key>CFBundleDisplayName</key>
    <string>Inwentaryzacja</string>
    <key>CFBundleVersion</key>
    <string>$APP_VERSION</string>
    <key>CFBundleShortVersionString</key>
    <string>$APP_VERSION</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleSignature</key>
    <string>????</string>
    <key>LSMinimumSystemVersion</key>
    <string>13.0</string>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSRequiresAquaSystemAppearance</key>
    <false/>
EOF

# Dodaj informacje o architekturze
if echo "$APP_ARCHS" | grep -q "universal\|x86_64.*arm64\|arm64.*x86_64"; then
cat >> "$TEMP_DIR/$APP_BUNDLE/Contents/Info.plist" <<EOF
    <key>LSArchitecturePriority</key>
    <array>
        <string>arm64</string>
        <string>x86_64</string>
    </array>
EOF
fi

cat >> "$TEMP_DIR/$APP_BUNDLE/Contents/Info.plist" <<EOF
</dict>
</plist>
EOF

# ==========================
# Kopiowanie plików aplikacji
# ==========================
echo "📂 Kopiowanie plików aplikacji..."

# Główna aplikacja
cp "$DEPLOY_DIR/$APP_NAME" "$TEMP_DIR/$APP_BUNDLE/Contents/MacOS/"

# Biblioteki
cp "$DEPLOY_DIR"/*.dylib "$TEMP_DIR/$APP_BUNDLE/Contents/Frameworks/" 2>/dev/null || true

# Pluginy
cp -r "$DEPLOY_DIR/platforms" "$TEMP_DIR/$APP_BUNDLE/Contents/PlugIns/" 2>/dev/null || true
cp -r "$DEPLOY_DIR/sqldrivers" "$TEMP_DIR/$APP_BUNDLE/Contents/PlugIns/" 2>/dev/null || true
cp -r "$DEPLOY_DIR/imageformats" "$TEMP_DIR/$APP_BUNDLE/Contents/PlugIns/" 2>/dev/null || true

# Ikona (jeśli istnieje)
if [[ -f "images/app_icon.icns" ]]; then
  cp "images/app_icon.icns" "$TEMP_DIR/$APP_BUNDLE/Contents/Resources/"
  echo "    <key>CFBundleIconFile</key>" >> "$TEMP_DIR/$APP_BUNDLE/Contents/Info.plist.tmp"
  echo "    <string>app_icon</string>" >> "$TEMP_DIR/$APP_BUNDLE/Contents/Info.plist.tmp"
elif [[ -f "images/icon.icns" ]]; then
  cp "images/icon.icns" "$TEMP_DIR/$APP_BUNDLE/Contents/Resources/"
fi

# ==========================
# Naprawienie ścieżek w bundle
# ==========================
echo "🔧 Naprawianie ścieżek w bundle..."

# Funkcja do naprawy install_name w bundle
fix_bundle_paths() {
  local target_file=$1
  local is_executable=${2:-false}
  
  if [[ ! -f "$target_file" ]]; then
    return
  fi
  
  echo "🔧 Bundle fix: $(basename "$target_file")"
  
  otool -L "$target_file" | awk 'NR>1 {print $1}' | while read -r lib; do
    lib_name=$(basename "$lib")
    
    # Sprawdź czy biblioteka jest w Frameworks
    if [[ -f "$TEMP_DIR/$APP_BUNDLE/Contents/Frameworks/$lib_name" ]]; then
      if [[ "$is_executable" == true ]]; then
        install_name_tool -change "$lib" "@executable_path/../Frameworks/$lib_name" "$target_file" 2>/dev/null || true
      else
        install_name_tool -change "$lib" "@loader_path/../Frameworks/$lib_name" "$target_file" 2>/dev/null || true
      fi
    fi
  done
}

# Napraw główną aplikację
fix_bundle_paths "$TEMP_DIR/$APP_BUNDLE/Contents/MacOS/$APP_NAME" true

# Napraw biblioteki
for lib_file in "$TEMP_DIR/$APP_BUNDLE/Contents/Frameworks"/*.dylib; do
  if [[ -f "$lib_file" ]]; then
    fix_bundle_paths "$lib_file" false
  fi
done

# Napraw pluginy
for plugin_file in "$TEMP_DIR/$APP_BUNDLE/Contents/PlugIns"/*/*.dylib; do
  if [[ -f "$plugin_file" ]]; then
    fix_bundle_paths "$plugin_file" false
  fi
done

# ==========================
# Dodanie dokumentacji i licencji
# ==========================
echo "📄 Dodawanie dokumentacji..."

if [[ -f "README.md" ]]; then
  cp "README.md" "$TEMP_DIR/"
fi

if [[ -f "LICENSE" ]]; then
  cp "LICENSE" "$TEMP_DIR/"
fi

if [[ -f "CHANGELOG.md" ]]; then
  cp "CHANGELOG.md" "$TEMP_DIR/"
fi

# ==========================
# Tworzenie symlinku do Applications
# ==========================
ln -sf /Applications "$TEMP_DIR/Applications"

# ==========================
# Utworzenie tła DMG (opcjonalne)
# ==========================
if [[ -f "images/dmg_background.png" ]]; then
  cp "images/dmg_background.png" "$TEMP_DIR/.background.png"
fi

# ==========================
# Tworzenie DMG
# ==========================
echo "📦 Tworzenie obrazu DMG..."

# Usunięcie istniejącego DMG
rm -f "$DMG_FILENAME"

# Obliczenie rozmiaru
SIZE_MB=$(du -sm "$TEMP_DIR" | cut -f1)
SIZE_MB=$((SIZE_MB + 50))  # Dodaj 50MB marginesu

echo "💾 Rozmiar DMG: ${SIZE_MB}MB"

# Tworzenie tymczasowego obrazu
hdiutil create -srcfolder "$TEMP_DIR" -volname "$APP_NAME $APP_VERSION" -fs HFS+ \
  -fsargs "-c c=64,a=16,e=16" -format UDRW -size ${SIZE_MB}m "temp_$DMG_FILENAME"

# Montowanie dla customizacji
MOUNT_POINT="/Volumes/$APP_NAME $APP_VERSION"
hdiutil attach "temp_$DMG_FILENAME" -readwrite -noverify -noautoopen

# Customizacja wyglądu (jeśli możliwe)
if command -v osascript >/dev/null 2>&1; then
  echo "🎨 Customizacja wyglądu DMG..."
  
osascript <<EOF
tell application "Finder"
    tell disk "$APP_NAME $APP_VERSION"
        open
        set current view of container window to icon view
        set toolbar visible of container window to false
        set statusbar visible of container window to false
        set the bounds of container window to {400, 100, 920, 440}
        set theViewOptions to the icon view options of container window
        set arrangement of theViewOptions to not arranged
        set icon size of theViewOptions to 72
        set background picture of theViewOptions to file ".background.png"
        set position of item "$APP_BUNDLE" of container window to {160, 205}
        set position of item "Applications" of container window to {360, 205}
        close
        open
        update without registering applications
        delay 2
    end tell
end tell
EOF
fi

# Odmontowanie
hdiutil detach "$MOUNT_POINT"

# Konwersja do finalnego formatu (skompresowany, read-only)
echo "🗜️  Kompresowanie DMG..."
hdiutil convert "temp_$DMG_FILENAME" -format UDZO -imagekey zlib-level=9 -o "$DMG_FILENAME"

# Czyszczenie
rm -f "temp_$DMG_FILENAME"
rm -rf "$TEMP_DIR"

# ==========================
# Weryfikacja finalnego DMG
# ==========================
echo -e "\n✅ DMG utworzony pomyślnie!"
echo "📦 Plik: $DMG_FILENAME"
echo "💾 Rozmiar: $(du -h "$DMG_FILENAME" | cut -f1)"

# Sprawdzenie zawartości
echo "📋 Sprawdzanie zawartości DMG..."
hdiutil attach "$DMG_FILENAME" -readonly -noverify -noautoopen -mountpoint "/tmp/verify_dmg"

if [[ -f "/tmp/verify_dmg/$APP_BUNDLE/Contents/MacOS/$APP_NAME" ]]; then
  APP_ARCHS_DMG=$(lipo -archs "/tmp/verify_dmg/$APP_BUNDLE/Contents/MacOS/$APP_NAME" 2>/dev/null || echo "unknown")
  echo "✅ Aplikacja w DMG: $APP_ARCHS_DMG"
else
  echo "❌ Problem z aplikacją w DMG"
fi

hdiutil detach "/tmp/verify_dmg"

echo -e "\n🎉 Pakiet DMG gotowy do dystrybucji!"
echo "📤 Możesz teraz udostępnić plik: $DMG_FILENAME"
