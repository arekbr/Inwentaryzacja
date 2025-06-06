#!/bin/bash
set -e

echo -e "\n🔄 [RESTORE PLUGINS] Przywracanie Qt SQL Plugins z kopii zapasowej\n"

# ==========================
# Wczytanie konfiguracji
# ==========================
if [[ ! -f qt_env_universal.sh ]]; then
  echo "❌ Brak pliku qt_env_universal.sh. Uruchom najpierw bootstrap."
  exit 1
fi

source qt_env_universal.sh

# ==========================
# Funkcja restore dla jednej instalacji Qt
# ==========================
restore_qt_plugins() {
  local qt_path=$1
  local arch_name=$2
  
  echo "🔄 Przywracanie pluginów SQL w $qt_path ($arch_name)..."
  
  PLUGIN_DIR="$qt_path/plugins/sqldrivers"
  if [[ ! -d "$PLUGIN_DIR" ]]; then
    echo "⚠️  Katalog $PLUGIN_DIR nie istnieje - pomijam"
    return 0
  fi
  
  # Znajdź najnowszą kopię zapasową
  BACKUP_DIR=$(find "$qt_path/plugins" -name "sqldrivers_backup_*" -type d | sort | tail -n1)
  
  if [[ -z "$BACKUP_DIR" ]]; then
    echo "❌ Nie znaleziono kopii zapasowej w $qt_path/plugins/"
    echo "💡 Szukam wzorców: sqldrivers_backup_*"
    find "$qt_path/plugins" -name "*backup*" -type d 2>/dev/null || true
    return 1
  fi
  
  echo "📁 Znaleziono kopię zapasową: $BACKUP_DIR"
  echo "⚠️  To działanie zastąpi obecne pluginy SQL!"
  
  read -p "❓ Kontynuować przywracanie dla $arch_name? (y/n) " answer
  if [[ "$answer" != "y" ]]; then
    echo "🚪 Pominięto $arch_name"
    return 0
  fi
  
  # Usuń obecne pluginy i przywróć z kopii
  rm -rf "$PLUGIN_DIR"
  cp -r "$BACKUP_DIR" "$PLUGIN_DIR"
  
  echo "✅ Przywrócono pluginy SQL dla $arch_name"
  echo "📋 Przywrócone pliki:"
  ls -la "$PLUGIN_DIR/"
}

# ==========================
# Główna logika
# ==========================
if [[ "$QT_DUAL_ARCH" == "true" ]]; then
  echo "🔄 Dual-arch setup wykryty"
  restore_qt_plugins "$QT_PATH_INTEL" "Intel"
  restore_qt_plugins "$QT_PATH_ARM" "ARM"
else
  restore_qt_plugins "$QT_PATH" "$BUILD_TYPE"
fi

echo -e "\n✅ Przywracanie pluginów SQL zakończone!"
echo "💡 Jeśli potrzebujesz przebudować plugin MySQL, uruchom:"
echo "   ./2.build_qt_mysql_plugin_macos_universal.sh"
