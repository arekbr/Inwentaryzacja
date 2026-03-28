param(
    [string]$BuildDir = 'build-windows',
    [string]$OutputDir = 'dist\windows',
    [string]$AppName = 'Inwentaryzacja',
    [string]$Version = ''
)

$ErrorActionPreference = 'Stop'

$RootDir = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($Version)) {
    $Version = (Get-Content (Join-Path $RootDir 'version.txt') -Raw).Trim()
}

$QtPath = (& qmake -query QT_INSTALL_PREFIX).Trim()
$QtPlugins = (& qmake -query QT_INSTALL_PLUGINS).Trim()
$WinDeployQt = Join-Path $QtPath 'bin\windeployqt.exe'

if (-not (Test-Path $WinDeployQt)) {
    throw "Nie znaleziono windeployqt: $WinDeployQt"
}

$BuildExe = Join-Path $RootDir "$BuildDir\$AppName.exe"
if (-not (Test-Path $BuildExe)) {
    $BuildExe = Join-Path $RootDir "$BuildDir\Release\$AppName.exe"
}

if (-not (Test-Path $BuildExe)) {
    throw "Nie znaleziono binarki: $AppName.exe"
}

$AbsoluteOutputDir = Join-Path $RootDir $OutputDir
$StageRoot = Join-Path $AbsoluteOutputDir 'stage'
$DeployDir = Join-Path $StageRoot $AppName

Remove-Item $StageRoot -Recurse -Force -ErrorAction Ignore
New-Item -ItemType Directory -Path $DeployDir -Force | Out-Null

$DeployedExe = Join-Path $DeployDir "$AppName.exe"
Copy-Item $BuildExe $DeployedExe -Force

& $WinDeployQt $DeployedExe --release --no-translations --compiler-runtime | Write-Host

$SqlDriversDir = Join-Path $DeployDir 'sqldrivers'
New-Item -ItemType Directory -Path $SqlDriversDir -Force | Out-Null

$QtSqlite = Join-Path $QtPlugins 'sqldrivers\qsqlite.dll'
if (Test-Path $QtSqlite) {
    Copy-Item $QtSqlite $SqlDriversDir -Force
}

$QtMysql = Join-Path $QtPlugins 'sqldrivers\qsqlmysql.dll'
if (Test-Path $QtMysql) {
    Copy-Item $QtMysql $SqlDriversDir -Force
}

$MariadbLibrary = Get-ChildItem -Path $QtPath -Filter 'libmariadb*.dll' -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
if ($MariadbLibrary) {
    Copy-Item $MariadbLibrary.FullName $DeployDir -Force
}

New-Item -ItemType Directory -Path $AbsoluteOutputDir -Force | Out-Null

$ZipPath = Join-Path $AbsoluteOutputDir ("{0}_{1}_Windows_x64.zip" -f $AppName, $Version)
if (Test-Path $ZipPath) {
    Remove-Item $ZipPath -Force
}
Compress-Archive -Path (Join-Path $DeployDir '*') -DestinationPath $ZipPath -Force

$InnoScript = Join-Path $RootDir 'ci\Inwentaryzacja.iss'
$Iscc = (Get-Command ISCC.exe -ErrorAction SilentlyContinue).Source
if (-not $Iscc) {
    $DefaultIscc = 'C:\Program Files (x86)\Inno Setup 6\ISCC.exe'
    if (Test-Path $DefaultIscc) {
        $Iscc = $DefaultIscc
    }
}

if (-not $Iscc) {
    throw 'Nie znaleziono ISCC.exe (Inno Setup).'
}

& $Iscc "/DMyAppVersion=$Version" "/DMyAppSource=$DeployDir" "/DMyAppOutput=$AbsoluteOutputDir" $InnoScript | Write-Host

Write-Host "✅ Gotowe pakiety w: $AbsoluteOutputDir" -ForegroundColor Green
Get-ChildItem $AbsoluteOutputDir | Format-Table Name, Length -AutoSize
