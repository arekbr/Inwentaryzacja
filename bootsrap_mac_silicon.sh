#!/bin/bash
set -e

###############################################################################
#  Bootstrap script for macOS (Apple Silicon M‑series)
#  ‑ Installs Homebrew dependencies
#  ‑ Sets up vcpkg in manifest mode (libmariadb + openssl)
#  ‑ Re‑uses an existing binary Qt (recommended) or can build Qt from source
#  ‑ Builds & installs the Inwentaryzacja project into build/install/
###############################################################################

##### ───────────────────── User‑tunable variables ────────────────────────── ###
PROJECT_DIR="$PWD"
VCPKG_DIR="$PROJECT_DIR/vcpkg"

# Point to an *installed* Qt6 for Apple Silicon. Use Qt Online Installer.
QT_BIN="${QT_BIN:-$HOME/Qt/6.6.2/macos}"   # change if you installed a different version
QT_VERSION="6.6.2"                           # used only if we fall back to source build

QT_SRC_DIR="$PROJECT_DIR/qt-src"
QT_BUILD_DIR="$PROJECT_DIR/qt-build"
QT_INSTALL_DIR="${QT_BIN:-$PROJECT_DIR/qt}"

# vcpkg triplet for macOS/arm64
VCPKG_TRIPLET="arm64-osx"

##### ───────────────────── Helper functions ─────────────────────────────── ###
fail() { echo "ERROR: $1"; exit 1; }

check_tools() {
  echo "▶ Checking command‑line tools…"
  for cmd in git cmake ninja clang curl tar brew; do
    command -v "$cmd" >/dev/null || fail "$cmd not found – install Xcode & Homebrew first"
  done
  echo "✔ Tools OK"
}

install_brew_deps() {
  echo "▶ Installing Homebrew packages…"
  brew update
  brew install pkg-config icu4c mariadb-connector-c openssl@3 ninja
  echo "✔ Homebrew deps installed"
}

setup_vcpkg() {
  if [ ! -d "$VCPKG_DIR" ]; then
    echo "▶ Cloning vcpkg…"
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
  else
    echo "▶ Updating vcpkg…"
    git -C "$VCPKG_DIR" pull
  fi
  "$VCPKG_DIR/bootstrap-vcpkg.sh" -disableMetrics

  # Update builtin‑baseline in vcpkg.json
  BASELINE=$(git -C "$VCPKG_DIR" rev-parse HEAD)
  if grep -q '"builtin-baseline"' vcpkg.json; then
    sed -i '' "s/\"builtin-baseline\": \".*\"/\"builtin-baseline\": \"$BASELINE\"/" vcpkg.json
  else
    sed -i '' "\/\"version-string\"/a\
  \ \ \ \ \"builtin-baseline\": \"$BASELINE\"," vcpkg.json
  fi

  echo "▶ Installing vcpkg ports (manifest)…"
  "$VCPKG_DIR/vcpkg" install --triplet "$VCPKG_TRIPLET"
}

build_qt_from_source() {
  if [ -n "$QT_BIN" ] && [ -d "$QT_BIN/lib" ]; then
    echo "▶ Using binary Qt at $QT_BIN (skip build)"
    return
  fi

  echo "⚠️  No binary Qt found – building Qt $QT_VERSION from source (long)"
  [[ -d "$QT_SRC_DIR" ]] || {
    curl -L "https://download.qt.io/archive/qt/${QT_VERSION%.*}/${QT_VERSION}/single/qt-everywhere-src-${QT_VERSION}.tar.xz" -o qt.tar.xz
    tar -xf qt.tar.xz && mv "qt-everywhere-src-${QT_VERSION}" "$QT_SRC_DIR" && rm qt.tar.xz
  }

  mkdir -p "$QT_BUILD_DIR" && cd "$QT_BUILD_DIR"
  "$QT_SRC_DIR/configure" \
     -prefix "$QT_INSTALL_DIR" \
     -opensource -confirm-license -release \
     -nomake examples -nomake tests \
     -skip qtquick3d -skip qtgraphs -skip qtquick3dphysics \
     -framework -qt-pcre -qt-harfbuzz -securetransport

  cmake --build . --parallel "$(sysctl -n hw.logicalcpu)"
  cmake --install .
  cd "$PROJECT_DIR"
}

build_project() {
  echo "▶ Configuring CMake project…"
  mkdir -p build && cd build
  cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET" \
    -DCMAKE_PREFIX_PATH="$QT_INSTALL_DIR" \
    -DCMAKE_BUILD_TYPE=Release

  echo "▶ Building…"
  cmake --build . --parallel "$(sysctl -n hw.logicalcpu)"
  echo "▶ Installing…"
  cmake --install .
  cd "$PROJECT_DIR"
}

##### ───────────────────── Main flow ─────────────────────────────────────── ###
check_tools
install_brew_deps
setup_vcpkg
build_qt_from_source
build_project

echo -e "\n🎉  Build finished!\nRun with:\n  ./build/install/bin/Inwentaryzacja.sh"
