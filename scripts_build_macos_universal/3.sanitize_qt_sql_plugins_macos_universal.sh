#!/bin/bash
set -e

echo -e "\n🧹 [SANITIZE UNIVERSAL] Czyszczenie SQL Plugins (macOS Universal)\n"

# ==========================
# Wczytanie konfiguracji
# ==========================
if [[ ! -f qt_env_universal.sh ]]; then
  echo "❌ Brak pliku qt_env_universal.sh. Uruchom najpierw bootstrap."
  exit 1
fi

source qt_env_universal.sh

# ==========================
# Funkcja sanitize dla jednej instalacji Qt
# ==========================
sanitize_qt_plugins() {
  local qt_path=$1
  local arch_name=$2
  
  echo "🧹 Czyszczenie pluginów SQL w $qt_path ($arch_name)..."
  
  PLUGIN_DIR="$qt_path/plugins/sqldrivers"
  if [[ ! -d "$PLUGIN_DIR" ]]; then
    echo "⚠️  Katalog $PLUGIN_DIR nie istnieje - pomijam"
    return 0
  fi
  
  BACKUP_DIR="$qt_path/plugins/sqldrivers_backup_$(date +%Y%m%d_%H%M%S)"
  
  echo "💾 Tworzenie kopii zapasowej: $BACKUP_DIR"
  cp -r "$PLUGIN_DIR" "$BACKUP_DIR"
  
  # Lista potencjalnie problematycznych pluginów na macOS
  PROBLEMATIC_PLUGINS=(
    "libqsqlodbc.dylib"    # Często brakuje ODBC drivers
    "libqsqlpsql.dylib"    # PostgreSQL - może wymagać dodatkowych dependencies
    "libqsqlibase.dylib"   # Firebird - rzadko używany
    "libqsqloci.dylib"     # Oracle - wymaga Oracle Client
    "libqsqldb2.dylib"     # IBM DB2 - rzadko używany
  )
  
  for plugin in "${PROBLEMATIC_PLUGINS[@]}"; do
    plugin_path="$PLUGIN_DIR/$plugin"
    if [[ -f "$plugin_path" ]]; then
      echo "🔍 Sprawdzam $plugin..."
      
      # Test czy plugin się ładuje
      if otool -L "$plugin_path" | grep -q "not found\|missing"; then
        echo "❌ $plugin ma brakujące dependencies - usuwam"
        rm "$plugin_path"
      else
        echo "✅ $plugin wygląda OK - zostawiam"
      fi
    fi
  done
  
  echo "✅ Sanityzacja zakończona dla $arch_name"
}

# ==========================
# Główna logika
# ==========================
if [[ "$QT_DUAL_ARCH" == "true" ]]; then
  sanitize_qt_plugins "$QT_PATH_INTEL" "Intel"
  sanitize_qt_plugins "$QT_PATH_ARM" "ARM"
else
  sanitize_qt_plugins "$QT_PATH" "$BUILD_TYPE"
fi

echo -e "\n✅ Sanityzacja pluginów SQL zakończona!"
echo "➡️  Kolejny krok: ./4.build_inwentaryzacja_macos_universal.sh"
