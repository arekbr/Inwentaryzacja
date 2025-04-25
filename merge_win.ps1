# Nazwa pliku wynikowego
$outputFile = "combined_sources.txt"

# Czyścimy plik jeśli istnieje
"" | Out-File -FilePath $outputFile -Encoding UTF8

# Szukane rozszerzenia
$extensions = "*.pro", "*.h", "*.cpp", "*.ui", "*.bat", "*.ps1", "*.sh", "*.sql"

# Pętla po rozszerzeniach
foreach ($ext in $extensions) {
    Get-ChildItem -Path . -Recurse -Filter $ext -File | Where-Object {
        # Pomijamy wszystkie pliki, które są w folderze "build"
        $_.FullName -notmatch "\\build\\"
    } | ForEach-Object {
        $file = $_.FullName
        Add-Content -Path $outputFile -Value "========"
        Add-Content -Path $outputFile -Value "Start of $($_.Name)"
        Add-Content -Path $outputFile -Value "========"
        Get-Content -Path $file | Add-Content -Path $outputFile
        Add-Content -Path $outputFile -Value "`n====="
        Add-Content -Path $outputFile -Value "End of file $($_.Name)"
        Add-Content -Path $outputFile -Value "====="
        Add-Content -Path $outputFile -Value ""
    }
}

Write-Host "Gotowe! Plik wynikowy: $outputFile"
