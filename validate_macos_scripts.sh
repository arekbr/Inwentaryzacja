#!/bin/bash

echo "🔍 Walidacja składni skryptów macOS Universal..."

SCRIPT_DIR="scripts_build_macos_universal"
ERRORS=0

for script in "$SCRIPT_DIR"/*.sh; do
    if [[ -f "$script" ]]; then
        script_name=$(basename "$script")
        echo -n "Sprawdzam $script_name... "
        
        if bash -n "$script" 2>/dev/null; then
            echo "✅ OK"
        else
            echo "❌ BŁĄD SKŁADNI"
            echo "Szczegóły błędu:"
            bash -n "$script"
            echo ""
            ((ERRORS++))
        fi
    fi
done

echo ""
if [[ $ERRORS -eq 0 ]]; then
    echo "🎉 Wszystkie skrypty mają poprawną składnię!"
    exit 0
else
    echo "❌ Znaleziono $ERRORS błędów składni!"
    exit 1
fi
