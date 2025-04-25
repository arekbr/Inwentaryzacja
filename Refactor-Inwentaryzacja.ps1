# ================================
# Refactor-Inwentaryzacja.ps1
# ================================
# Skrypt porządkujący strukturę Qt projektu "Inwentaryzacja"

param(
    [string]$SRC_DIR = "C:\Users\Arek\projektyQT\Inwentaryzacja",
    [string]$DST_DIR = "C:\zmiany_inwent"
)

# Wyklucz katalogi tymczasowe kompilatora
$excludedDirs = @("build", "debug", "release", "moc", "moc_*", "x64", "x86", "out", "tmp")

# Filtr pomocniczy – funkcja
function IsNotExcluded($path) {
    foreach ($ex in $excludedDirs) {
        if ($path -like "*\$ex\*") { return $false }
    }
    return $true
}

# Tworzymy wymagane katalogi
$folders = @("src", "include", "forms", "translations", "docs", "build", "images")
foreach ($folder in $folders) {
    $target = Join-Path $DST_DIR $folder
    if (-not (Test-Path $target)) {
        New-Item -ItemType Directory -Path $target | Out-Null
    }
}

# .cpp → src/
Get-ChildItem -Path $SRC_DIR -Recurse -Include *.cpp -File | Where-Object {
    IsNotExcluded($_.FullName)
} | ForEach-Object {
    Copy-Item $_.FullName -Destination (Join-Path $DST_DIR "src") -Force
}

# .h → include/
Get-ChildItem -Path $SRC_DIR -Recurse -Include *.h -File | Where-Object {
    IsNotExcluded($_.FullName)
} | ForEach-Object {
    Copy-Item $_.FullName -Destination (Join-Path $DST_DIR "include") -Force
}

# .ui → forms/
Get-ChildItem -Path $SRC_DIR -Recurse -Include *.ui -File | Where-Object {
    IsNotExcluded($_.FullName)
} | ForEach-Object {
    Copy-Item $_.FullName -Destination (Join-Path $DST_DIR "forms") -Force
}

# .ts → translations/
Get-ChildItem -Path $SRC_DIR -Recurse -Include *.ts -File | Where-Object {
    IsNotExcluded($_.FullName)
} | ForEach-Object {
    Copy-Item $_.FullName -Destination (Join-Path $DST_DIR "translations") -Force
}

# .pro, .sh, .bat, .ps1, .sql, .md, .qrc → katalog główny
$rootFiles = "*.pro", "*.md", "*.sql", "*.sh", "*.bat", "*.ps1", "*.qrc"
foreach ($pattern in $rootFiles) {
    Get-ChildItem -Path $SRC_DIR -Recurse -Include $pattern -File | Where-Object {
        IsNotExcluded($_.FullName)
    } | ForEach-Object {
        Copy-Item $_.FullName -Destination $DST_DIR -Force
    }
}

# images/
$imagesSrc = Join-Path $SRC_DIR "images"
if (Test-Path $imagesSrc) {
    Copy-Item $imagesSrc -Destination $DST_DIR -Recurse -Force
}

Write-Host "Struktura została utworzona zgodnie z plikiem .pro i bez śmieci builda!" -ForegroundColor Green
Write-Host "Gotowe: $DST_DIR" -ForegroundColor Cyan