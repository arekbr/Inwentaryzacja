#!/bin/bash
set -e

echo -e "\n🧹 [CLEAN UNIVERSAL] Czyszczenie wszystkich plików build (macOS Universal)\n"

# ==========================
# Wczytanie konfiguracji (opcjonalne)
# ==========================
if [[ -f qt_env_universal.sh ]]; then
  source qt_env_universal.sh
  echo "ℹ️  Wczytano konfigurację z qt_env_universal.sh"
  echo "📋 BUILD_TYPE: ${BUILD_TYPE:-"nieznany"}"
fi

# ==========================
# Listowanie co będzie usunięte
# ==========================
echo "🔍 Skanowanie plików do usunięcia..."

DIRS_TO_CLEAN=(
  "build_inwentaryzacja_universal"
  "build_inwentaryzacja_intel" 
  "build_inwentaryzacja_arm"
  "build_inwentaryzacja_native"
  "build_qt_sql_drivers_universal"
  "build_qt_sql_drivers_intel"
  "build_qt_sql_drivers_arm"
  "build_qt_sql_drivers_native"
  "temp_dmg"
  "deploy"
)

FILES_TO_CLEAN=(
  "temp_*.dmg"
  "Inwentaryzacja_*.dmg"
  "*.app"
)

echo "📁 Katalogi do usunięcia:"
for dir in "${DIRS_TO_CLEAN[@]}"; do
  if [[ -d "$dir" ]]; then
    SIZE=$(du -sh "$dir" 2>/dev/null | cut -f1 || echo "?")
    echo "   🗂️  $dir ($SIZE)"
  fi
done

echo "📄 Pliki do usunięcia:"
for pattern in "${FILES_TO_CLEAN[@]}"; do
  for file in $pattern; do
    if [[ -f "$file" ]]; then
      SIZE=$(du -sh "$file" 2>/dev/null | cut -f1 || echo "?")
      echo "   📄 $file ($SIZE)"
    fi
  done
done

# Sprawdź łączny rozmiar
TOTAL_SIZE=0
for dir in "${DIRS_TO_CLEAN[@]}"; do
  if [[ -d "$dir" ]]; then
    DIR_SIZE=$(du -sm "$dir" 2>/dev/null | cut -f1 || echo "0")
    TOTAL_SIZE=$((TOTAL_SIZE + DIR_SIZE))
  fi
done

for pattern in "${FILES_TO_CLEAN[@]}"; do
  for file in $pattern; do
    if [[ -f "$file" ]]; then
      FILE_SIZE=$(du -sm "$file" 2>/dev/null | cut -f1 || echo "0")
      TOTAL_SIZE=$((TOTAL_SIZE + FILE_SIZE))
    fi
  done
done

if [[ $TOTAL_SIZE -gt 0 ]]; then
  echo "💾 Łączny rozmiar do zwolnienia: ~${TOTAL_SIZE}MB"
else
  echo "✅ Brak plików do usunięcia"
  exit 0
fi

# ==========================
# Potwierdzenie usunięcia
# ==========================
echo -e "\n⚠️  UWAGA: To działanie nieodwracalnie usunie wszystkie pliki build!"
echo "💡 Zachowane zostaną:"
echo "   • Pliki źródłowe projektu"
echo "   • qt_env_universal.sh (konfiguracja)"
echo "   • Skrypty w scripts_build_macos_universal/"

read -p "❓ Czy na pewno chcesz kontynuować? (yes/no) " answer
if [[ "$answer" != "yes" ]]; then
  echo "🚪 Anulowano czyszczenie"
  exit 0
fi

# ==========================
# Czyszczenie katalogów
# ==========================
echo -e "\n🧹 Usuwanie katalogów build..."

for dir in "${DIRS_TO_CLEAN[@]}"; do
  if [[ -d "$dir" ]]; then
    echo "🗑️  Usuwanie: $dir"
    rm -rf "$dir"
  fi
done

# ==========================
# Czyszczenie plików
# ==========================
echo "🧹 Usuwanie plików tymczasowych..."

for pattern in "${FILES_TO_CLEAN[@]}"; do
  for file in $pattern; do
    if [[ -f "$file" ]]; then
      echo "🗑️  Usuwanie: $file"
      rm -f "$file"
    fi
  done
done

# ==========================
# Opcjonalne czyszczenie konfiguracji
# ==========================
if [[ -f "qt_env_universal.sh" ]]; then
  echo -e "\n❓ Czy chcesz również usunąć plik konfiguracji qt_env_universal.sh?"
  echo "💡 Usunięcie tego pliku wymagać będzie ponownego uruchomienia bootstrap"
  
  read -p "❓ Usunąć qt_env_universal.sh? (y/n) " answer
  if [[ "$answer" == "y" ]]; then
    rm -f qt_env_universal.sh
    echo "🗑️  Usunięto: qt_env_universal.sh"
  else
    echo "✅ Zachowano: qt_env_universal.sh"
  fi
fi

# ==========================
# Podsumowanie
# ==========================
echo -e "\n✅ Czyszczenie zakończone!"

# Sprawdź co zostało
REMAINING_BUILD_DIRS=$(find . -maxdepth 1 -name "build_*" -type d | wc -l | tr -d ' ')
REMAINING_DMG_FILES=$(find . -maxdepth 1 -name "*.dmg" -type f | wc -l | tr -d ' ')

if [[ $REMAINING_BUILD_DIRS -eq 0 && $REMAINING_DMG_FILES -eq 0 ]]; then
  echo "🎉 Wszystkie pliki build zostały usunięte"
else
  echo "ℹ️  Pozostało:"
  echo "   📁 Katalogi build: $REMAINING_BUILD_DIRS"
  echo "   📦 Pliki DMG: $REMAINING_DMG_FILES"
  
  if [[ $REMAINING_BUILD_DIRS -gt 0 ]]; then
    echo "📁 Pozostałe katalogi build:"
    find . -maxdepth 1 -name "build_*" -type d
  fi
  
  if [[ $REMAINING_DMG_FILES -gt 0 ]]; then
    echo "📦 Pozostałe pliki DMG:"
    find . -maxdepth 1 -name "*.dmg" -type f
  fi
fi

echo -e "\n💡 Aby rozpocząć nowy build:"
if [[ ! -f "qt_env_universal.sh" ]]; then
  echo "   ./1.bootstrap_macos_universal.sh"
else
  echo "   source qt_env_universal.sh"
  echo "   ./4.build_inwentaryzacja_macos_universal.sh  # (lub od kroku 2 dla MySQL)"
fi
