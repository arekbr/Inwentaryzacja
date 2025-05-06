#!/bin/bash

# Pobierz listę plików i posortuj je według daty/godziny w nazwie
files=$(ls PXL_*.jpg | sort)

# Licznik dla nowych nazw
counter=1

# Przetwarzaj każdy plik
for file in $files; do
    # Wyodrębnij datę i godzinę z nazwy pliku
    base_name=$(echo "$file" | sed 's/\..*//') # Usuń rozszerzenie i wszystko po nim
    
    # Stwórz nową nazwę z licznikiem
    new_name="${counter}.jpg"
    
    # Zmień nazwę pliku
    mv -v "$file" "$new_name"
    
    # Zwiększ licznik
    counter=$((counter + 1))
done

echo "Przemianowanie zakończone!"
