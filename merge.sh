#!/bin/bash

# Nazwa pliku wynikowego
outputFile="combined_sources.txt"

# Czyścimy plik jeśli istnieje
> "$outputFile"

# Szukane rozszerzenia
extensions=("*.pro" "*.h" "*.cpp" "*.ui" "*.bat" "*.ps1" "*.sh" "*.sql")

# Funkcja sprawdzająca czy plik jest w katalogu 'build'
is_in_build_dir() {
    [[ "$1" == *"/build/"* ]]
}

# Pętla po rozszerzeniach
for ext in "${extensions[@]}"; do
    find . -type f -name "$ext" | while read -r file; do
        # Pomijamy pliki w folderze build
        if ! is_in_build_dir "$file"; then
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
