$ErrorActionPreference = "Stop"

$APP_NAME = "Inwentaryzacja"
$VERSION = "1.2.4"
$DEPLOY_DIR = "deploy"
$INSTALLER_DIR = "installer"
$TOOLS_DIR = "tools"
$QT_PATH = "C:\Qt\6.9.0\mingw_64"
$QTIFW_ZIP_URL = "https://github.com/qtproject/installer-framework/archive/refs/heads/4.9.zip"
$QTIFW_ZIP = "$TOOLS_DIR\qtifw.zip"
$QTIFW_ROOT = $null
$INSTALLERBASE_EXE = "$TOOLS_DIR\installerbase.exe"
$RCC_PATH = "$QT_PATH\bin\rcc.exe"
$QMAKE_PATH = "$QT_PATH\bin\qmake.exe"
$MAKE_PATH = "C:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe"

# Ustawienie PATH, aby priorytet miało MinGW z Qt
$Env:PATH = "C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.9.0\mingw_64\bin;" + $Env:PATH

# =============================
# Przygotowanie katalogów
# =============================
New-Item -ItemType Directory -Path $TOOLS_DIR -Force | Out-Null

# =============================
# Budowa installerbase.exe, jeśli nie istnieje
# =============================
if (!(Test-Path $INSTALLERBASE_EXE)) {
    Write-Host "📥 Brak installerbase.exe — budowanie z QtIFW 4.9..."

    # Usuwanie starych wersji frameworka
    Get-ChildItem -Path $TOOLS_DIR -Directory |
        Where-Object { $_.Name -like "installer-framework-*" } |
        ForEach-Object {
            Write-Host "🧹 Usuwanie: $($_.FullName)"
            Remove-Item -Recurse -Force $_.FullName
        }

    Write-Host "📦 Pobieranie Qt Installer Framework..."
    if (Test-Path $QTIFW_ZIP) {
        Remove-Item -Force $QTIFW_ZIP
    }

    Invoke-WebRequest -Uri $QTIFW_ZIP_URL -OutFile $QTIFW_ZIP
    Expand-Archive -LiteralPath $QTIFW_ZIP -DestinationPath $TOOLS_DIR -Force

    # Automatyczne wykrycie katalogu głównego QtIFW (z plikiem installerfw.pro)
    $QTIFW_ROOT = Get-ChildItem -Path $TOOLS_DIR -Recurse -Directory -ErrorAction SilentlyContinue |
        Where-Object { Test-Path (Join-Path $_.FullName "installerfw.pro") } |
        Select-Object -First 1 -ExpandProperty FullName

    if (-not $QTIFW_ROOT -or !(Test-Path $QTIFW_ROOT)) {
        Write-Error "❌ Nie znaleziono katalogu z installerfw局 się błędem."
        exit 1
    }

    # Przygotowanie resources.qrc
    $RES_QRC = "$QTIFW_ROOT\resources.qrc"
    @"
<RCC>
  <qresource prefix="/">
    <file>installericon.ico</file>
    <file>logo.png</file>
    <file>background.png</file>
    <file>watermark.png</file>
    <file>default.qss</file>
  </qresource>
</RCC>
"@ | Set-Content -Encoding UTF8 $RES_QRC

    # Kopiowanie zasobów z właściwych katalogów
    $assetSources = @{
        "installericon.ico" = "images\installericon.ico"
        "logo.png"          = "images\logo.png"
        "background.png"    = "images\background.png"
        "watermark.png"     = "images\watermark.png"
        "default.qss"       = "styles\default.qss"
    }

    foreach ($key in $assetSources.Keys) {
        $src = $assetSources[$key]
        $dst = Join-Path $QTIFW_ROOT $key
        if (!(Test-Path $src)) {
            Write-Error "❌ Brakuje pliku: $src"
            exit 1
        }
        Copy-Item $src $dst -Force
    }

    # Patchowanie błędu QHash<QString, QFlags<...>> w commandlineparser.h
    $parserHeader = Join-Path $QTIFW_ROOT "src\libs\installer\commandlineparser.h"
    if (Test-Path $parserHeader) {
        Write-Host "🩹 Patchowanie: $parserHeader"
        $content = Get-Content $parserHeader -Raw

        # Dodaj brakujące includy na początku pliku, po strażniku nagłówkowym
        $includesToAdd = @(
            "#include <QHash>",
            "#include <QFlags>",
            "#include <QMetaType>",
            "#include <QString>"
        )
        if ($content -match '#ifndef\s+\w+_H') {
            $includeInsertPoint = ($content -split '\n' | Select-String '#define\s+\w+_H').LineNumber
            foreach ($inc in $includesToAdd) {
                if (-not ($content -match [regex]::Escape($inc))) {
                    $content = $content -replace "(#define\s+\w+_H[^\n]*\n)", "`$1$inc`n"
                }
            }
        } else {
            Write-Warning "⚠️ Nie znaleziono strażnika nagłówkowego w: $parserHeader"
            foreach ($inc in $includesToAdd) {
                if (-not ($content -match [regex]::Escape($inc))) {
                    $content = $inc + "`n" + $content
                }
            }
        }

        # Usuń istniejące deklaracje Q_DECLARE_TYPEINFO i Q_DECLARE_METATYPE, aby zapobiec redefinicji
        $content = $content -replace 'Q_DECLARE_TYPEINFO\s*\(\s*CommandLineParser::OptionContextFlag\s*,\s*Q_PRIMITIVE_TYPE\s*\)\s*;\s*\n?', ''
        $content = $content -replace 'Q_DECLARE_METATYPE\s*\(\s*QFlags\s*<\s*CommandLineParser::OptionContextFlag\s*>\s*\)\s*;\s*\n?', ''

        # Znajdź definicję enum OptionContextFlag i dodaj Q_DECLARE_FLAGS
        if ($content -match 'enum\s+OptionContextFlag\s*{[^}]*}') {
            $enumDefinition = $Matches[0]
            if (-not ($content -match 'Q_DECLARE_FLAGS\s*\(\s*OptionContextFlags\s*,\s*OptionContextFlag\s*\)')) {
                $content = $content -replace [regex]::Escape($enumDefinition), "$enumDefinition`nQ_DECLARE_FLAGS(OptionContextFlags, OptionContextFlag)`nQ_DECLARE_OPERATORS_FOR_FLAGS(OptionContextFlags)"
            }
        } else {
            Write-Warning "⚠️ Nie znaleziono definicji enum OptionContextFlag w: $parserHeader"
        }

        # Dodaj Q_DECLARE_TYPEINFO i Q_DECLARE_METATYPE po Q_DECLARE_FLAGS, ale przed końcem strażnika nagłówkowego
        $flagsDeclaration = 'Q_DECLARE_TYPEINFO(CommandLineParser::OptionContextFlag, Q_PRIMITIVE_TYPE);'
        $metaDeclaration = 'Q_DECLARE_METATYPE(QFlags<CommandLineParser::OptionContextFlag>);'
        if ($content -match 'Q_DECLARE_FLAGS\s*\(\s*OptionContextFlags\s*,\s*OptionContextFlag\s*\)') {
            $content = $content -replace '(Q_DECLARE_OPERATORS_FOR_FLAGS\s*\(\s*OptionContextFlags\s*\))', "`$1`n$flagsDeclaration`n$metaDeclaration"
        } else {
            # Jeśli Q_DECLARE_FLAGS nie istnieje, dodaj deklaracje na końcu pliku, przed #endif
            if ($content -match '#endif') {
                $content = $content -replace '(#endif.*)', "$flagsDeclaration`n$metaDeclaration`n`$1"
            } else {
                $content += "`n$flagsDeclaration`n$metaDeclaration"
            }
        }

        # Zapisz zmiany
        Set-Content -Path $parserHeader -Value $content -Encoding UTF8
        Write-Host "✅ Załatano: commandlineparser.h"
    } else {
        Write-Error "❌ Nie znaleziono pliku: $parserHeader"
        exit 1
    }

    # Patchowanie błędu konfliktu definicji mode_t w abstractarchive.h
    $archiveHeader = Join-Path $QTIFW_ROOT "src\libs\installer\abstractarchive.h"
    if (Test-Path $archiveHeader) {
        Write-Host "🩹 Patchowanie: $archiveHeader"
        Write-Host "Zawartość abstractarchive.h przed patchowaniem:"
        Get-Content $archiveHeader | Write-Host

        $content = Get-Content $archiveHeader -Raw

        # Zastąp definicję mode_t warunkiem, aby uniknąć konfliktu
        $modeTDefinition = 'typedef\s+int\s+mode_t\s*;'
        $conditionalModeT = "#ifndef mode_t`ntypedef int mode_t;`n#endif"
        if ($content -match $modeTDefinition) {
            $content = $content -replace $modeTDefinition, $conditionalModeT
        } else {
            Write-Warning "⚠️ Nie znaleziono definicji 'typedef int mode_t;' w: $archiveHeader"
            # Spróbuj bardziej elastycznego dopasowania
            $modeTDefinitionLoose = 'typedef\s+int\s+mode_t\s*;\s*\n'
            if ($content -match $modeTDefinitionLoose) {
                $content = $content -replace $modeTDefinitionLoose, $conditionalModeT + "`n"
            } else {
                Write-Warning "⚠️ Drugie podejście do dopasowania 'typedef int mode_t;' również nie powiodło się."
            }
        }

        # Zapisz zmiany
        Set-Content -Path $archiveHeader -Value $content -Encoding UTF8
        Write-Host "Zawartość abstractarchive.h po patchowaniu:"
        Get-Content $archiveHeader | Write-Host
        Write-Host "✅ Załatano: abstractarchive.h"
    } else {
        Write-Error "❌ Nie znaleziono pliku: $archiveHeader"
        exit 1
    }

    # Budowanie installerfw.pro
    Push-Location $QTIFW_ROOT
    Write-Host "🧹 Czyszczenie projektu..."
    # Pełne czyszczenie: usuń katalog release i uruchom distclean, jeśli dostępne
    if (Test-Path "release") {
        Remove-Item -Recurse -Force "release"
    }
    & $MAKE_PATH clean
    if (Test-Path "Makefile") {
        & $MAKE_PATH distclean -ErrorAction SilentlyContinue
    }
    Write-Host "🚀 Budowanie installerbase.exe..."
    & $QMAKE_PATH installerfw.pro
    if ($LASTEXITCODE -ne 0) {
        Write-Error "❌ qmake zakończył się błędem."
        exit 1
    }
    & $MAKE_PATH
    if ($LASTEXITCODE -ne 0) {
        Write-Error "❌ mingw32-make zakończył się błędem."
        exit 1
    }
    Pop-Location

    $builtExe = "$QTIFW_ROOT\bin\installerbase.exe"
    if (!(Test-Path $builtExe)) {
        Write-Error "❌ Nie znaleziono zbudowanego installerbase.exe"
        exit 1
    }
    Copy-Item $builtExe $INSTALLERBASE_EXE -Force
    Write-Host "`n✅ Gotowe: $INSTALLERBASE_EXE"
} else {
    Write-Host "✅ installerbase.exe już istnieje: $INSTALLERBASE_EXE"
}

# Tutaj można kontynuować: budowa zasobów RCC i finalnego instalatora