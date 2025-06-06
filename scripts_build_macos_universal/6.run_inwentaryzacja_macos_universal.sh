#!/bin/bash
set -e

APP_NAME="Inwentaryzacja"
DEPLOY_DIR="deploy"

echo -e "\n🚀 [RUN UNIVERSAL] Uruchamianie aplikacji (macOS Universal)\n"

# ==========================
# Sprawdzenie czy deployment istnieje
# ==========================
if [[ ! -f "$DEPLOY_DIR/$APP_NAME" ]]; then
  echo "❌ Nie znaleziono aplikacji w $DEPLOY_DIR/$APP_NAME"
  echo "🔧 Uruchom najpierw: ./5.deploy_macos_universal.sh"
  exit 1
fi

# ==========================
# Informacje o aplikacji
# ==========================
echo "📊 Informacje o aplikacji:"
echo "📁 Lokalizacja: $DEPLOY_DIR/$APP_NAME"

APP_ARCHS=$(lipo -archs "$DEPLOY_DIR/$APP_NAME" 2>/dev/null || echo "unknown")
echo "🏗️  Architektury: $APP_ARCHS"

HOST_ARCH=$(uname -m)
echo "💻 Architektura systemu: $HOST_ARCH"

# Sprawdź kompatybilność
if [[ "$APP_ARCHS" == "unknown" ]]; then
  echo "⚠️  Nie można określić architektury aplikacji"
elif echo "$APP_ARCHS" | grep -q "$HOST_ARCH"; then
  echo "✅ Aplikacja jest kompatybilna z tym systemem"
elif echo "$APP_ARCHS" | grep -q "universal\|fat"; then
  echo "✅ Aplikacja jest universal binary"
else
  echo "⚠️  Aplikacja może nie być kompatybilna (app: $APP_ARCHS, system: $HOST_ARCH)"
  
  if [[ "$HOST_ARCH" == "x86_64" && "$APP_ARCHS" == "arm64" ]]; then
    echo "❌ Aplikacja ARM64 nie może działać na Intel Mac bez Rosetta"
    exit 1
  elif [[ "$HOST_ARCH" == "arm64" && "$APP_ARCHS" == "x86_64" ]]; then
    echo "🔄 Aplikacja Intel będzie uruchomiona przez Rosetta"
  fi
fi

# ==========================
# Sprawdzenie zależności
# ==========================
echo -e "\n🔍 Sprawdzanie zależności..."

missing_deps=false

# Sprawdź czy wszystkie biblioteki są dostępne
otool -L "$DEPLOY_DIR/$APP_NAME" | awk 'NR>1 {print $1}' | while read -r lib; do
  if [[ "$lib" =~ ^@executable_path/ ]]; then
    local_lib="${lib/@executable_path\//$DEPLOY_DIR/}"
    if [[ ! -f "$local_lib" ]]; then
      echo "❌ Brakuje biblioteki: $local_lib"
      missing_deps=true
    fi
  elif [[ "$lib" =~ ^(/usr/local/|/opt/homebrew/) ]]; then
    if [[ ! -f "$lib" ]]; then
      echo "⚠️  Brakuje zewnętrznej biblioteki: $lib"
      echo "   💡 Możliwe rozwiązanie: brew install $(basename "$lib" | sed 's/^lib//;s/\.dylib.*//')"
    fi
  fi
done

# Sprawdź pluginy
for plugin in "$DEPLOY_DIR/platforms"/*.dylib "$DEPLOY_DIR/sqldrivers"/*.dylib; do
  if [[ -f "$plugin" ]]; then
    plugin_name=$(basename "$plugin")
    plugin_archs=$(lipo -archs "$plugin" 2>/dev/null || echo "unknown")
    
    if [[ "$plugin_archs" != "unknown" ]]; then
      if [[ "$APP_ARCHS" =~ "universal" && ! "$plugin_archs" =~ "$HOST_ARCH" ]]; then
        echo "⚠️  Plugin $plugin_name może mieć problemy z architekturą ($plugin_archs)"
      fi
    fi
  fi
done

# ==========================
# Sprawdzenie uprawnień
# ==========================
if [[ ! -x "$DEPLOY_DIR/$APP_NAME" ]]; then
  echo "🔧 Ustawianie uprawnień wykonywania..."
  chmod +x "$DEPLOY_DIR/$APP_NAME"
fi

# ==========================
# Sprawdzenie Gatekeeper (kod signing)
# ==========================
echo -e "\n🔐 Sprawdzanie podpisu kodu..."
if codesign -dv "$DEPLOY_DIR/$APP_NAME" 2>/dev/null; then
  echo "✅ Aplikacja jest podpisana"
else
  echo "⚠️  Aplikacja nie jest podpisana"
  echo "💡 Na macOS Ventura+ może być potrzebne:"
  echo "   xattr -cr '$DEPLOY_DIR'"
  echo "   Lub: System Preferences → Security & Privacy → Allow"
fi

# ==========================
# Opcje uruchamiania
# ==========================
echo -e "\n🎮 Opcje uruchamiania:"
echo "1) Uruchom aplikację normalnie"
echo "2) Uruchom z debugowaniem Qt"
echo "3) Sprawdź tylko czy aplikacja się uruchamia (test)"
echo "4) Anuluj"

read -p "❓ Wybierz opcję (1-4) [1]: " run_option
run_option="${run_option:-1}"

case $run_option in
  1)
    echo "🚀 Uruchamianie aplikacji..."
    cd "$DEPLOY_DIR"
    exec ./run_$APP_NAME.sh
    ;;
  2)
    echo "🐛 Uruchamianie z debugowaniem Qt..."
    cd "$DEPLOY_DIR"
    export QT_DEBUG_PLUGINS=1
    export QT_LOGGING_RULES="*.debug=true"
    exec ./run_$APP_NAME.sh
    ;;
  3)
    echo "🧪 Test uruchamiania..."
    cd "$DEPLOY_DIR"
    
    # Uruchom aplikację w tle na 3 sekundy
    timeout 3s ./run_$APP_NAME.sh &
    app_pid=$!
    
    sleep 1
    
    if kill -0 $app_pid 2>/dev/null; then
      echo "✅ Aplikacja uruchomiła się pomyślnie"
      kill $app_pid 2>/dev/null || true
    else
      echo "❌ Aplikacja nie uruchomiła się lub zakończyła się błędem"
      exit 1
    fi
    ;;
  4)
    echo "🚪 Anulowano"
    exit 0
    ;;
  *)
    echo "❌ Nieprawidłowy wybór"
    exit 1
    ;;
esac
