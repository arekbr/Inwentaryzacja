#!/bin/bash

# Nazwa pliku wynikowego
OUTPUT_FILE="output.txt"

# Usuń plik wynikowy, jeśli już istnieje, aby zacząć od nowa
if [ -f "$OUTPUT_FILE" ]; then
    rm "$OUTPUT_FILE"
fi

# Znajdź wszystkie pliki z podanymi rozszerzeniami w bieżącym katalogu
for file in $(find . -maxdepth 1 -type f \( -name "*.pro" -o -name "*.cpp" -o -name "*.ui" -o -name "*.h" -o -name "*.sql" \))
do
    # Pomiń katalogi i pliki wynikowe
    if [ -f "$file" ]; then
        # Pobierz nazwę pliku z pełną ścieżką, ale bez katalogu nadrzędnego
        filename=$(basename "$file")
        
        # Dodaj nazwę pliku do pliku wynikowego
        echo "$filename" >> "$OUTPUT_FILE"
        
        # Dodaj zawartość pliku do pliku wynikowego
        cat "$file" >> "$OUTPUT_FILE"
        
        # Dodaj pustą linię jako odstęp
        echo "" >> "$OUTPUT_FILE"
    fi
done

echo "Zakończono. Wynik zapisano do $OUTPUT_FILE"
