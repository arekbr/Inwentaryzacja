# Requires -Version 5.0
param(
    [string]$QtDir      = "C:\Qt\6.9.0\mingw_64",  # empty → build from source
    [string]$QtVersion  = "6.9.0",
    [switch]$Msvc       = $false                      # default MinGW
)

$ErrorActionPreference = 'Stop'
$ProjectDir = Get-Location
$VcpkgDir   = "$ProjectDir\vcpkg"
$QtSrcDir   = "$ProjectDir\qt-src"
$QtBuildDir = "$ProjectDir\qt-build"
$QtInstall  = if ($QtDir) { $QtDir } else { "$ProjectDir\qt" }
$Triplet    = if ($Msvc) { 'x64-windows' } else { 'x64-mingw-dynamic' }

function Fail($m){ Write-Host "ERROR: $m" -ForegroundColor Red; exit 1 }

function Ensure-Choco {
    if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
        Write-Host 'Installing Chocolatey…'
        Set-ExecutionPolicy Bypass -Scope Process -Force
        Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
    }
}

function Install-SystemDeps {
    Write-Host 'Installing system dependencies via choco…'
    $pkgs = @('git', 'cmake', 'ninja', 'pkgconfiglite',
              'openssl', 'mariadb-connector-c')
    if ($Msvc) {
        $pkgs += 'visualstudio2022buildtools'
    } else {
        $pkgs += 'mingw'
    }
    choco install -y $pkgs
}

function Setup-Vcpkg {
    if (-not (Test-Path $VcpkgDir)) { git clone https://github.com/microsoft/vcpkg.git $VcpkgDir }
    & "$VcpkgDir\bootstrap-vcpkg.bat" -disableMetrics
    $baseline = (git -C $VcpkgDir rev-parse HEAD).Trim()
    (Get-Content vcpkg.json) -replace '"builtin-baseline":\s*"[^"]+"', "\"builtin-baseline\": \"$baseline\"" | Set-Content vcpkg.json
    & "$VcpkgDir\vcpkg.exe" install --triplet $Triplet
}

function Build-QtFromSource {
    if ($QtDir) { Write-Host "Using Qt at $QtDir"; return }
    Write-Host "Building Qt $QtVersion from source (slow)…"
    $url = "https://download.qt.io/archive/qt/$($QtVersion.Substring(0,3))/$QtVersion/single/qt-everywhere-src-$QtVersion.zip"
    Invoke-WebRequest $url -OutFile qt.zip
    Expand-Archive qt.zip -DestinationPath . -Force
    Rename-Item "qt-everywhere-src-$QtVersion" qt-src
    New-Item -ItemType Directory $QtBuildDir -Force | Out-Null
    Push-Location $QtBuildDir
    & "$QtSrcDir\configure.bat" -prefix $QtInstall -opensource -confirm-license -nomake examples -nomake tests -release -skip qtquick3d -skip qtgraphs -skip qtquick3dphysics -no-warnings-are-errors
    cmake --build . --config Release -- /m
    cmake --install . --config Release
    Pop-Location
}

function Build-Project {
    New-Item -ItemType Directory build -Force | Out-Null
    Push-Location build
    $toolchain = "$VcpkgDir\scripts\buildsystems\vcpkg.cmake"
    cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=$toolchain -DVCPKG_TARGET_TRIPLET=$Triplet -DCMAKE_PREFIX_PATH=$QtInstall
    cmake --build . --config Release -- /m
    cmake --install . --config Release
    Pop-Location
}

Ensure-Choco
Install-SystemDeps
Setup-Vcpkg
Build-QtFromSource
Build-Project
Write-Host "`nBUILD FINISHED. Launch via build\install\bin\Inwentaryzacja.bat" -ForegroundColor Green
