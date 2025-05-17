<# ============================================================================
    BUILD SCRIPT – 100% CLEAN
    Tworzy pełny InwentaryzacjaInstaller.exe
    z ikoną instalatora, tłem, logo, watermarkiem i poprawnymi polskimi znakami
============================================================================ #>

$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# 1. Ścieżki i pliki (dopasowane do projektu)
# ---------------------------------------------------------------------------

$Root          = Split-Path -Parent $MyInvocation.MyCommand.Path
$Out           = "$Root\installer"
$AppSrc        = "$Root\deploy"
$InstallerExe  = "$Root\release_exe\InwentaryzacjaInstaller.exe"

$IFW           = 'C:\Qt\Tools\QtInstallerFramework\4.9'
$BinaryCreator = "$IFW\bin\binarycreator.exe"

# Pliki graficzne dla instalatora
$InstallerIconIco = "$Root\images\installericon.ico"
# $LogoPng          = "$Root\images\logo.png"
# $WatermarkPng     = "$Root\images\watermark.png"
# $BackgroundPng    = "$Root\images\background.png"
Write-Host "`n>>> Gotowe: 1. Ścieżki i pliki" -ForegroundColor Green

# ---------------------------------------------------------------------------
# 2. Przygotowanie folderów
# ---------------------------------------------------------------------------

# Odczytanie wersji z pliku version.txt
$version = Get-Content "$Root\version.txt" -Raw
$version = $version.Trim()

# Usuwamy stare wyniki
Remove-Item $Out, $InstallerExe -Recurse -Force -ErrorAction Ignore

# Tworzymy katalog bazowy i podkatalogi
New-Item -Path $Out -ItemType Directory -Force | Out-Null
New-Item -Path "$Out\config" -ItemType Directory -Force | Out-Null
New-Item -Path "$Out\packages\org.smok.inwentaryzacja\meta" -ItemType Directory -Force | Out-Null
New-Item -Path "$Out\packages\org.smok.inwentaryzacja\data" -ItemType Directory -Force | Out-Null

# <-- TU DODAJEMY
# Tworzymy katalog, do którego będziemy kopiować InwentaryzacjaInstaller.exe (i później interbase.dll)
New-Item -Path "$Root\release_exe" -ItemType Directory -Force | Out-Null

Write-Host "`n>>> Gotowe: 2. Przygotowanie folderów" -ForegroundColor Green

# ---------------------------------------------------------------------------
# 3. Kopiowanie plików aplikacji
# ---------------------------------------------------------------------------

# Kopiujemy aplikację do struktury instalatora
Copy-Item "$AppSrc\*" "$Out\packages\org.smok.inwentaryzacja\data\" -Recurse -Force

Write-Host "`n>>> Gotowe: 3. Kopiowanie plików aplikacji" -ForegroundColor Green

# ---------------------------------------------------------------------------
# 4. Tworzenie config.xml
# ---------------------------------------------------------------------------

# Konfiguracja główna instalatora
$config = @"
<?xml version="1.0"?>
<Installer>
    <Name>Inwentaryzacja</Name>
    <Version>$version</Version>
    <Title>Instalator Inwentaryzacja</Title>
    <Publisher>Stowarzyszenie Mi&#x142;o&#x15B;nik&#xF3;w Oldschoolowych Komputer&#xF3;w SMOK &amp; ChatGPT &amp; GROK</Publisher>

    <!-- Ikony aplikacji instalatora -->
    <InstallerWindowIcon>installericon</InstallerWindowIcon>
    <InstallerApplicationIcon>installericon</InstallerApplicationIcon>

    <!-- Pliki graficzne -->
    <!-- Logo>logo.png</Logo -->
    <!-- Watermark>watermark.png</Watermark -->
    <!-- Background>background.png</Background -->

    <!-- Parametry uruchamiania aplikacji po instalacji -->
    <RunProgram>@TargetDir@/Inwentaryzacja.exe</RunProgram>
    <RunProgramDescription>Inwentaryzacja retro komputer&#xF3;w</RunProgramDescription>

    <!-- Katalogi docelowe -->
    <StartMenuDir>Inwentaryzacja</StartMenuDir>
    <TargetDir>@HomeDir@/Inwentaryzacja</TargetDir>
    <AdminTargetDir>@RootDir@/Inwentaryzacja</AdminTargetDir>

    <AllowNonAsciiCharacters>true</AllowNonAsciiCharacters>

</Installer>
"@

# Zapisujemy config.xml w UTF-8 bez BOM
[IO.File]::WriteAllText("$Out\config\config.xml", $config, [Text.UTF8Encoding]::new($false))

Write-Host "`n>>> Gotowe: 4. Tworzenie config.xml" -ForegroundColor Green

# ---------------------------------------------------------------------------
# 5. Tworzenie package.xml
# ---------------------------------------------------------------------------

# Definicja pakietu
$package = @"
<?xml version="1.0"?>
<Package>
    <DisplayName xml:lang="pl_pl">Inwentaryzacja</DisplayName>
    <Description xml:lang="pl_pl">
        Program do inwentaryzacji retro komputer&#xF3;w
    </Description>
    <DisplayName xml:lang="en_us">Inventory</DisplayName>
    <Description xml:lang="en_us">Retro computer inventory tool</Description>
    <Version>$version</Version>
    <ReleaseDate>2025-04-26</ReleaseDate>
    <Default>true</Default>
    <Script>installscript.qs</Script>
</Package>
"@

[IO.File]::WriteAllText("$Out\packages\org.smok.inwentaryzacja\meta\package.xml", $package, [Text.UTF8Encoding]::new($false))

Write-Host "`n>>> Gotowe: 5. Tworzenie package.xml" -ForegroundColor Green

# ---------------------------------------------------------------------------
# 6. Tworzenie installscript.qs
# ---------------------------------------------------------------------------

# Skrypt tworzący skróty na pulpicie i w menu Start
$installScript = @'
function Component() {}

Component.prototype.createOperations = function() {
    component.createOperations();
    component.addOperation("CreateShortcut",
        "@TargetDir@/Inwentaryzacja.exe",
        "@DesktopDir@/Inwentaryzacja.lnk",
        "workingDirectory=@TargetDir@");
    component.addOperation("CreateShortcut",
        "@TargetDir@/Inwentaryzacja.exe",
        "@StartMenuDir@/Inwentaryzacja.lnk",
        "workingDirectory=@TargetDir@");
};
'@

[IO.File]::WriteAllText("$Out\packages\org.smok.inwentaryzacja\meta\installscript.qs", $installScript, [Text.UTF8Encoding]::new($false))

Write-Host "`n>>> Gotowe: 6. Tworzenie installscript.qs" -ForegroundColor Green

# ---------------------------------------------------------------------------
# 7. Kopiowanie plików graficznych
# ---------------------------------------------------------------------------

# Qt Installer Framework będzie szukał plików graficznych w katalogu głównym instalatora
Copy-Item $InstallerIconIco "$Out\config\installericon.ico" -Force
#Copy-Item $LogoPng          "$Out\config\logo.png" -Force
#Copy-Item $WatermarkPng     "$Out\config\watermark.png" -Force
#Copy-Item $BackgroundPng    "$Out\config\background.png" -Force

Write-Host "`n>>> Gotowe: 7. Kopiowanie plików graficznych" -ForegroundColor Green

# ---------------------------------------------------------------------------
# 8. Budowanie instalatora
# ---------------------------------------------------------------------------

& "$BinaryCreator" -c "$Out\config\config.xml" `
                   -p "$Out\packages" `
                   "$InstallerExe"

Write-Host "`n>>> Gotowe: 8. Budowanie instalatora" -ForegroundColor Green

# ---------------------------------------------------------------------------
# 9. Gotowe!
# ---------------------------------------------------------------------------

Write-Host "`n>>> Gotowe: $InstallerExe" -ForegroundColor Green
