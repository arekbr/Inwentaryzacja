<#  ============================================================================
    BUILD  SCRIPT   –   100 %  ASCII
    tworzy   InwentaryzacjaInstaller.exe   z poprawną ścieżką  i  polskimi
    znakami  oraz  zmienioną  ikoną
============================================================================ #>

$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# 1.  Funkcje pomocnicze
# ---------------------------------------------------------------------------

# zapisuje tekst w UTF-8 bez BOM
function Write-Utf8 { param($Path,$Text)
    $utf8 = [Text.UTF8Encoding]::new($false)   # $false = no BOM
    [IO.File]::WriteAllText($Path,$Text,$utf8)
}

# zamienia {HH} na znak Unicode:  U "00F3"  = ó
function U { param($hex) [char][Convert]::ToInt32($hex,16) }

# w szablonie zastępuje {HH} znakami Unicode
function Fmt-U {
    param([string]$raw)

    # ────────────────┐         ┌────────────── brak cudzysłowów
    return $raw -replace '\{([0-9A-Fa-f]{2,4})\}', {
        [char]([Convert]::ToInt32($args[0].Groups[1].Value,16))
    }
}
# ---------------------------------------------------------------------------
# 2.  Ścieżki (dopasuj do siebie)
# ---------------------------------------------------------------------------

$Root          = Split-Path -Parent $MyInvocation.MyCommand.Path
$Out           = "$Root\installer"
$AppSrc        = "$Root\gotowa"
$InstallerExe  = "$Root\InwentaryzacjaInstaller.exe"

$IFW           = 'C:\Qt\Tools\QtInstallerFramework\4.9'
$BinaryCreator = "$IFW\bin\binarycreator.exe"
$InstallerBase = "$IFW\bin\installerbase.exe"
$InstallerBaseCustom = "$Root\installerbase_custom.exe"

$IconIco       = "$Root\images\ikonawin.ico"
$ResHack       = 'C:\Program Files (x86)\Resource Hacker\ResourceHacker.exe'

# ---------------------------------------------------------------------------
# 3.  Sprzątanie + tworzenie pustej struktury
# ---------------------------------------------------------------------------

# kasujemy stare wyniki
Remove-Item $Out,$InstallerExe,$InstallerBaseCustom -Recurse -Force -EA Ignore

# NOWA LINIA – tworzymy katalog bazowy "installer"
New-Item -Path $Out -ItemType Directory -Force | Out-Null

# podkatalogi
New-Item -Path "$Out\config"                                   -ItemType Directory -Force | Out-Null
New-Item -Path "$Out\packages\org.smok.inwentaryzacja\meta"    -ItemType Directory -Force | Out-Null
New-Item -Path "$Out\packages\org.smok.inwentaryzacja\data"    -ItemType Directory -Force | Out-Null


# ---------------------------------------------------------------------------
# 4.  Kopiowanie plików aplikacji
# ---------------------------------------------------------------------------

Copy-Item "$AppSrc\*" "$Out\packages\org.smok.inwentaryzacja\data\" -Recurse -Force

# ---------------------------------------------------------------------------
# 5.  config.xml
# ---------------------------------------------------------------------------

$config = Fmt-U @"
<?xml version="1.0"?>
<Installer>
    <Name>Inwentaryzacja</Name>
    <Version>1.1.8</Version>
    <Title>Instalator Inwentaryzacja</Title>

    <Publisher>Stowarzyszenie Mi{0142}o{015B}nik{00F3}w Oldschoolowych Komputer{00F3}w SMOK &amp; ChatGPT &amp; GROK</Publisher>

    <StartMenuDir>Inwentaryzacja</StartMenuDir>
    <TargetDir>@HomeDir@/Inwentaryzacja</TargetDir>
    <AllowSpaceInPath>true</AllowSpaceInPath>
</Installer>
"@

Write-Utf8 "$Out\config\config.xml" $config

# ---------------------------------------------------------------------------
# 6.  package.xml  (PL + fallback EN)
# ---------------------------------------------------------------------------

$pkg = Fmt-U @"
<?xml version="1.0"?>
<Package>
    <DisplayName xml:lang="pl_pl">Inwentaryzacja</DisplayName>

    <Description xml:lang="pl_pl">
  Program do inwentaryzacji retro komputer&#xF3;w
</Description>

    <DisplayName xml:lang="en_us">Inventory</DisplayName>
    <Description  xml:lang="en_us">Retro computer inventory tool</Description>

    <Version>1.1.8</Version>
    <ReleaseDate>2025-04-26</ReleaseDate>
    <Default>true</Default>
    <Script>installscript.qs</Script>
</Package>
"@

Write-Utf8 "$Out\packages\org.smok.inwentaryzacja\meta\package.xml" $pkg

# ---------------------------------------------------------------------------
# 7.  installscript.qs
# ---------------------------------------------------------------------------

$qs = @'
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

Write-Utf8 "$Out\packages\org.smok.inwentaryzacja\meta\installscript.qs" $qs

# ---------------------------------------------------------------------------
# 8.  Tworzenie własnego installerbase z ikoną
# ---------------------------------------------------------------------------

Copy-Item $InstallerBase $InstallerBaseCustom -Force
& "$ResHack" -open "$InstallerBaseCustom" `
             -save "$InstallerBaseCustom" `
             -action addoverwrite `
             -res "$IconIco" `
             -mask "ICONGROUP,MAINICON,"

# ---------------------------------------------------------------------------
# 9.  Budowanie instalatora
# ---------------------------------------------------------------------------

& "$BinaryCreator" -c "$Out\config\config.xml" `
                   -p "$Out\packages" `
                   -t "$InstallerBaseCustom" `
                   "$InstallerExe"

# ---------------------------------------------------------------------------
# 9.  czyszczenie icon cache
# ---------------------------------------------------------------------------

& ie4uinit.exe -ClearIconCache

Write-Host "`n>>> Gotowe: $InstallerExe" -ForegroundColor Green
