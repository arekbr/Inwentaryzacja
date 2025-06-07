#!/bin/bash

echo "🧪 Test poprawek do Universal macOS Build Scripts"
echo "================================================="

# Test 1: Sprawdzenie czy bootstrap nie ma hardcoded 6.9.0/6.9.1
echo -e "\n1️⃣ Sprawdzenie bootstrap skryptu..."

if grep -q "6\.9\." "1.bootstrap_macos_universal.sh"; then
    echo "❌ BŁĄD: Znaleziono hardcoded wersję Qt 6.9.x w bootstrap"
    grep -n "6\.9\." "1.bootstrap_macos_universal.sh"
else
    echo "✅ Bootstrap: Brak hardcoded wersji Qt"
fi

# Test 2: Sprawdzenie inteligentnego wykrywania MariaDB w MySQL plugin
echo -e "\n2️⃣ Sprawdzenie MySQL plugin skryptu..."

if grep -q "MARIADB_CANDIDATES" "2.build_qt_mysql_plugin_macos_universal.sh"; then
    echo "✅ MySQL Plugin: Ma inteligentne wykrywanie MariaDB"
else
    echo "❌ BŁĄD: Brak inteligentnego wykrywania MariaDB"
fi

if grep -q "MARIADB_INCLUDE_CANDIDATES" "2.build_qt_mysql_plugin_macos_universal.sh"; then
    echo "✅ MySQL Plugin: Ma wykrywanie nagłówków MariaDB"
else
    echo "❌ BŁĄD: Brak wykrywania nagłówków MariaDB"
fi

# Test 3: Sprawdzenie czy nie ma prostych błędów składni
echo -e "\n3️⃣ Sprawdzenie składni skryptów..."

for script in *.sh; do
    if bash -n "$script" 2>/dev/null; then
        echo "✅ $script: Składnia OK"
    else
        echo "❌ $script: Błąd składni"
        bash -n "$script"
    fi
done

# Test 4: Sprawdzenie czy main skrypty mają odpowiednie uprawnienia
echo -e "\n4️⃣ Sprawdzenie uprawnień..."

for main_script in "1.bootstrap_macos_universal.sh" "2.build_qt_mysql_plugin_macos_universal.sh" "4.build_inwentaryzacja_macos_universal.sh"; do
    if [[ -x "$main_script" ]]; then
        echo "✅ $main_script: Wykonywalne"
    else
        echo "⚠️  $main_script: Nie jest wykonywalne (chmod +x potrzebne)"
    fi
done

echo -e "\n🎯 Podsumowanie testów zakończone!"
echo "💡 Jeśli wszystko OK, możesz przetestować:"
echo "   ./1.bootstrap_macos_universal.sh"
