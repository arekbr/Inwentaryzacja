#!/bin/bash
# QUICK START - Universal macOS Build

echo "🚀 QUICK START: Universal macOS Build dla Inwentaryzacja"
echo "======================================================="
echo ""

echo "📋 WYMAGANIA:"
echo "   ✅ macOS Ventura (13.0)+"
echo "   ✅ Xcode 15+ z Command Line Tools"
echo "   ✅ Homebrew"
echo "   ✅ Qt 6.9.x (universal lub dual-arch)"
echo ""

echo "🔄 SEKWENCJA POLECEŃ:"
echo "   1️⃣  chmod +x *.sh"
echo "   2️⃣  ./test_bootstrap.sh          # Test poprawności"
echo "   3️⃣  ./1.bootstrap_macos_universal.sh       # Konfiguracja"
echo "   4️⃣  source qt_env_universal.sh             # Załaduj env"
echo "   5️⃣  ./2.build_qt_mysql_plugin_macos_universal.sh  # MySQL plugin"
echo "   6️⃣  ./4.build_inwentaryzacja_macos_universal.sh    # Aplikacja"
echo "   7️⃣  ./5.deploy_macos_universal.sh          # Deployment"
echo "   8️⃣  ./6.run_inwentaryzacja_macos_universal.sh      # Test run"
echo "   9️⃣  ./7.make_dmg_macos_universal.sh        # Pakowanie DMG"
echo ""

echo "🎯 REZULTAT:"
echo "   📦 Inwentaryzacja-[VERSION]-universal.dmg"
echo "   🔄 Universal Binary (Intel + Apple Silicon)"
echo "   🍎 Gotowe do dystrybucji"
echo ""

echo "📘 DOKUMENTACJA:"
echo "   📄 README.md - Szczegółowa dokumentacja"
echo "   📋 PODSUMOWANIE_TESTOWANIE.md - Instrukcje dla testera"
echo ""

echo "💡 W razie problemów sprawdź pliki logów i README.md"
