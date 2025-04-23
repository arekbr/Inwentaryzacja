#!/bin/bash

# Nazwa pliku wynikowego
output_file="combined_sources.txt"

# Czyścimy plik jeśli istnieje
> "$output_file"

# Szukane rozszerzenia
extensions=("*.pro" "*.h" "*.cpp" "*.ui")

# Pętla po rozszerzeniach
for ext in "${extensions[@]}"; do
  for file in $ext; do
    if [[ -f "$file" ]]; then
      echo "========" >> "$output_file"
      echo "Start of $file" >> "$output_file"
      echo "========" >> "$output_file"
      cat "$file" >> "$output_file"
      echo -e "\n=====" >> "$output_file"
      echo "End of file $file" >> "$output_file"
      echo "=====" >> "$output_file"
      echo "" >> "$output_file"
    fi
  done
done

echo "Gotowe! Plik wynikowy: $output_file"

