@echo off
ECHO Setting up environment for Inwentaryzacja...

:: Funkcja wyświetlająca błąd
:fail
ECHO ERROR: %1
ECHO %2
EXIT /B 1

:: Sprawdzenie curl
WHERE curl >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    CALL :fail "curl not found" "Please install curl (e.g., download from https://curl.se/windows/)."
)
ECHO curl found

:: Sprawdzenie git
WHERE git >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    CALL :fail "git not found" "Please install git (e.g., download from https://git-scm.com/download/win)."
)
ECHO git found

:: Sprawdzenie CMake
WHERE cmake >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    ECHO Installing CMake...
    curl -L https://github.com/Kitware/CMake/releases/download/v3.30.3/cmake-3.30.3-windows-x86_64.zip -o cmake.zip
    tar -xf cmake.zip
    MOVE cmake-3.30.3-windows-x86_64 cmake
    SET "PATH=%CD%\cmake\bin;%PATH%"
)
ECHO CMake found

:: Sprawdzenie MinGW
WHERE g++ >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    CALL :fail "MinGW (g++) not found" "Please install MinGW (e.g., download from https://sourceforge.net/projects/mingw-w64/)."
)
ECHO Compiler (g++) found

:: Sprawdzenie Qt
SET QT_DIR=%CD%\qt
IF NOT EXIST "%QT_DIR%\bin\qmake.exe" (
    ECHO Downloading Qt 6.9.0...
    curl -L https://download.qt.io/official_releases/qt/6.9/6.9.0/single/qt-everywhere-src-6.9.0.zip -o qt.zip
    tar -xf qt.zip
    MOVE qt-everywhere-src-6.9.0 qt
    CD qt
    CALL configure.bat -prefix "%CD%" -opensource -confirm-license -nomake examples -nomake tests
    cmake --build . --parallel
    cmake --install .
    CD ..
)
SET "QT_DIR=%CD%\qt"
ECHO Qt found: %QT_DIR%

:: Konfiguracja vcpkg
SET VCPKG_DIR=%CD%\vcpkg
IF NOT EXIST "%VCPKG_DIR%" (
    ECHO Downloading vcpkg...
    git clone https://github.com/microsoft/vcpkg.git
    IF %ERRORLEVEL% NEQ 0 CALL :fail "Failed to clone vcpkg" "Check your internet connection or git installation."
    CD vcpkg
    CALL bootstrap-vcpkg.bat
    IF %ERRORLEVEL% NEQ 0 CALL :fail "Failed to bootstrap vcpkg" "Check vcpkg installation instructions at https://github.com/microsoft/vcpkg."
    CD ..
)
ECHO vcpkg ready at %VCPKG_DIR%

:: Instalacja zależności przez vcpkg
ECHO Installing dependencies with vcpkg...
%VCPKG_DIR%\vcpkg install --triplet x64-windows
IF %ERRORLEVEL% NEQ 0 CALL :fail "Failed to install dependencies" "Check vcpkg logs in %VCPKG_DIR%."

:: Budowanie projektu
ECHO Building Inwentaryzacja...
IF NOT EXIST build MKDIR build
CD build
cmake .. -DCMAKE_TOOLCHAIN_FILE=..\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_PREFIX_PATH="%QT_DIR%"
cmake --build . --config Release
cmake --install .
CD ..

ECHO Build complete! Application is in build\install\bin
ECHO Run it with: build\install\bin\Inwentaryzacja.exe