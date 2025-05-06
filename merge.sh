#!/bin/bash

# Nazwa pliku wynikowego
outputFile="combined_sources.txt"

# Czyścimy plik jeśli istnieje
> "$outputFile"

# Szukane rozszerzenia
extensions=("*.pro" "*.h" "*.cpp" "*.ui" "*.bat" "*.ps1" "*.sh" "*.sql")

# Lista katalogów do wykluczenia
excluded_dirs=("build" "deploy" "documentation" "vcpkg" "vcpkg_installed")  # Dodaj tutaj inne katalogi

# Funkcja sprawdzająca, czy plik jest w wykluczonym katalogu
is_in_excluded_dir() {
    for dir in "${excluded_dirs[@]}"; do
        if [[ "$1" == *"/$dir/"* ]]; then
            return 0  # Znaleziono w wykluczonym katalogu
        fi
    done
    return 1  # Nie znaleziono w wykluczonym katalogu
}

# Pętla po rozszerzeniach
for ext in "${extensions[@]}"; do
    find . -type f -name "$ext" | while read -r file; do
        # Pomijamy pliki w wykluczonych folderach
        if ! is_in_excluded_dir "$file"; then
            echo "========" >> "$outputFile"
            echo "Start of $(basename "$file")" >> "$outputFile"
            echo "========" >> "$outputFile"
            cat "$file" >> "$outputFile"
            echo "" >> "$outputFile"
            echo "=====" >> "$outputFile"
            echo "End of file $(basename "$file")" >> "$outputFile"
            echo "=====" >> "$outputFile"
            echo "" >> "$outputFile"
        fi
    done
done

echo "✅ Gotowe! Plik wynikowy: $outputFile"
