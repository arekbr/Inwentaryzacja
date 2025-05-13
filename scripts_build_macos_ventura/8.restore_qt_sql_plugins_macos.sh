#!/bin/bash
set -e

echo -e "\n♻️ [5a] Przywracanie pluginów SQL (pliki *.cmake)"

if [[ -z "$QT_PATH" && -f qt_env.sh ]]; then
    source qt_env.sh
fi

if [[ -z "$QT_PATH" ]]; then
    echo "❌ Nie ustawiono QT_PATH. Uruchom najpierw 'source qt_env.sh'"
    exit 1
fi

PLUGINS_DIR="$QT_PATH/lib/cmake/Qt6Sql"

for plugin in QODBC QPSQL QOCI QDB2 QIBASE QMIMER; do
    base="${PLUGINS_DIR}/Qt6${plugin}DriverPlugin"
    for suffix in Targets.cmake Config.cmake AdditionalTargetInfo.cmake; do
        bak="${base}${suffix}_bak"
        orig="${base}${suffix}"
        if [[ -f "$bak" ]]; then
            mv "$bak" "$orig"
            echo "↩️ Przywracam: ${orig##*/}"
        fi
    done
done

echo "✅ Gotowe: Qt przywrócone do stanu oryginalnego"
