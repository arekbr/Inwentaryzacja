#!/bin/bash

# Skrypt bootstrap dla Inwentaryzacja
# Automatyczna konfiguracja i budowanie projektu

echo "Setting up environment for Inwentaryzacja..."

# Funkcja wyświetlająca błąd i instrukcje
fail() {
    echo "ERROR: $1"
    echo "$2"
    exit 1
}

# Sprawdzenie curl
if ! command -v curl >/dev/null 2>&1; then
    fail "curl not found" \
         "Please install curl (e.g., 'sudo apt install curl' on Ubuntu, 'brew install curl' on macOS)."
fi
echo "curl found"

# Sprawdzenie git
if ! command -v git >/dev/null 2>&1; then
    fail "git not found" \
         "Please install git (e.g., 'sudo apt install git' on Ubuntu, 'brew install git' on macOS)."
fi
echo "git found"

# Sprawdzenie CMake
if ! command -v cmake >/dev/null 2>&1; then
    echo "Installing CMake..."
    curl -L https://github.com/Kitware/CMake/releases/download/v3.30.3/cmake-3.30.3-$(uname -s | tr '[:upper:]' '[:lower:]')-$(uname -m).tar.gz -o cmake.tar.gz
    tar -xzf cmake.tar.gz
    mv cmake-3.30.3-$(uname -s | tr '[:upper:]' '[:lower:]')-$(uname -m) cmake
    export PATH=$PWD/cmake/bin:$PATH
fi
echo "CMake found"

# Sprawdzenie kompilatora
if ! command -v g++ >/dev/null 2>&1; then
    fail "g++ not found" \
         "Please install g++ (e.g., 'sudo apt install build-essential' on Ubuntu, 'xcode-select --install' on macOS)."
fi
echo "Compiler (g++) found"

# Sprawdzenie Qt
QT_DIR="$PWD/qt"
if [ ! -d "$QT_DIR" ] || [ ! -x "$QT_DIR/bin/qmake" ]; then
    echo "Downloading Qt 6.9.0..."
    QT_URL="https://download.qt.io/official_releases/qt/6.9/6.9.0/single/qt-everywhere-src-6.9.0.tar.xz"
    curl -L "$QT_URL" -o qt.tar.xz
    tar -xJf qt.tar.xz
    mv qt-everywhere-src-6.9.0 qt
    cd qt
    ./configure -prefix "$PWD" -opensource -confirm-license -nomake examples -nomake tests
    cmake --build . --parallel
    cmake --install .
    cd ..
fi
export QT_DIR="$PWD/qt"
echo "Qt found: $QT_DIR"

# Konfiguracja vcpkg
VCPKG_DIR="$PWD/vcpkg"
if [ ! -d "$VCPKG_DIR" ]; then
    echo "Downloading vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git || fail "Failed to clone vcpkg" "Check your internet connection or git installation."
    cd vcpkg
    ./bootstrap-vcpkg.sh || fail "Failed to bootstrap vcpkg" "Check vcpkg installation instructions at https://github.com/microsoft/vcpkg."
    cd ..
fi
echo "vcpkg ready at $VCPKG_DIR"

# Instalacja zależności przez vcpkg
echo "Installing dependencies with vcpkg..."
$VCPKG_DIR/vcpkg install --triplet $(uname -m)-$(uname | tr '[:upper:]' '[:lower:]') || fail "Failed to install dependencies" "Check vcpkg logs in $VCPKG_DIR."

# Budowanie projektu
echo "Building Inwentaryzacja..."
mkdir -p build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_PREFIX_PATH="$QT_DIR"
make
make install

echo "Build complete! Application is in build/install"
if [ "$(uname -s)" = "Darwin" ]; then
    echo "Run it with: open build/install/Inwentaryzacja.app"
else
    echo "Run it with: build/install/bin/Inwentaryzacja"
fi