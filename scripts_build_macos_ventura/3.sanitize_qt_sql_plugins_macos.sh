#!/bin/bash
set -e

echo -e "\n🧼 [3a] Wyłączanie nieistniejących pluginów SQL (tymczasowo)"

if [[ -z "$QT_PATH" && -f qt_env.sh ]]; then
    source qt_env.sh
fi

if [[ -z "$QT_PATH" ]]; then
    echo "❌ Zmienna QT_PATH nie jest ustawiona."
    exit 1
fi

for plugin in QODBC QPSQL QOCI QDB2 QIBASE QMIMER; do
  base="$QT_PATH/lib/cmake/Qt6Sql/Qt6${plugin}DriverPlugin"
  plugin_lower=$(echo "$plugin" | tr '[:upper:]' '[:lower:]')
  dylib="$QT_PATH/plugins/sqldrivers/libqsql${plugin_lower}.dylib"
  if [[ ! -f "$dylib" ]]; then
    echo "🧹 Usuwam pliki CMake pluginu $plugin (brak $dylib)"
    for suffix in Targets.cmake Config.cmake AdditionalTargetInfo.cmake; do
      file="${base}${suffix}"
      if [[ -f "$file" ]]; then
        mv "$file" "${file}_bak"
        echo "   🔕 → ${file##*/} wyłączony"
      fi
    done
  fi
done

echo "✅ Gotowe: brakujące pluginy nie będą psuć konfiguracji projektu"
