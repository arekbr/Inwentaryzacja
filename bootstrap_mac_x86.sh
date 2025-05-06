#!/bin/bash
set -e

###############################################################################
#  Bootstrap script for macOS (Intel x86_64 and Apple Silicon M‑series)
#  - Installs Homebrew & required brew packages
#  - Sets up vcpkg (manifest mode) with libmariadb + openssl
#  - Re‑uses an existing binary Qt (recommended) or can build Qt from source
#  - Builds & installs the Inwentaryzacja project into build/install/
###############################################################################

PROJECT_DIR="$PWD"
VCPKG_DIR="$PROJECT_DIR/vcpkg"

# Point QT_BIN to your Qt installation (e.g. /Applications/Qt/6.6.3/macos)
QT_BIN="${QT_BIN:-}"            # leave empty to build Qt from source (slow!)
QT_VERSION="6.6.3"              # used only if building from source

QT_SRC_DIR="$PROJECT_DIR/qt-src"
QT_BUILD_DIR="$PROJECT_DIR/qt-build"
QT_INSTALL_DIR="${QT_BIN:-$PROJECT_DIR/qt}"

ARCH=$(uname -m)                # arm64 or x86_64
case "$ARCH" in
  arm64)   VCPKG_TRIPLET="arm64-osx" ; BREW_ARCH="--arm"   ;;
  x86_64)  VCPKG_TRIPLET="x64-osx"   ; BREW_ARCH="--intel" ;;
  *) echo "Unsupported macOS arch: $ARCH"; exit 1 ;;
esac

fail(){ echo "\033[31mERROR:\033[0m $1"; exit 1; }

check_cli(){
  for cmd in git cmake ninja clang make curl tar brew; do
    command -v "$cmd" >/dev/null || fail "$cmd not installed. Install Xcode CLT & Homebrew first."
  done
}

brew_deps(){
  echo "▶ Installing Homebrew packages …"
  brew install $BREW_ARCH \ 
      ninja pkg-config icu4c openssl@3 mariadb-connector-c qt@6 || true
}

setup_vcpkg(){
  if [ ! -d "$VCPKG_DIR" ]; then
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
  fi
  "$VCPKG_DIR/bootstrap-vcpkg.sh"
  BASELINE=$(git -C "$VCPKG_DIR" rev-parse HEAD)
  if grep -q '"builtin-baseline"' vcpkg.json; then
    sed -i '' "s/\"builtin-baseline\": \".*\"/\"builtin-baseline\": \"$BASELINE\"/" vcpkg.json
  else
    sed -i '' "/\"version-string\"/a \
  \"builtin-baseline\": \"$BASELINE\"," vcpkg.json
  fi
  "$VCPKG_DIR/vcpkg" install --triplet "$VCPKG_TRIPLET"
}

build_qt(){
  if [ -n "$QT_BIN" ]; then echo "▶ Using Qt at $QT_BIN"; return; fi
  # Download & build from source (slow!)
  curl -L "https://download.qt.io/archive/qt/${QT_VERSION%.*}/${QT_VERSION}/single/qt-everywhere-src-${QT_VERSION}.tar.xz" -o qt.tar.xz
  tar xf qt.tar.xz && mv qt-everywhere-src-${QT_VERSION} "$QT_SRC_DIR"
  mkdir -p "$QT_BUILD_DIR" && cd "$QT_BUILD_DIR"
  "$QT_SRC_DIR/configure" -prefix "$QT_INSTALL_DIR" -opensource -confirm-license \
      -nomake examples -nomake tests -release -skip qtquick3d -skip qtgraphs -skip qtquick3dphysics -no-warnings-are-errors
  cmake --build . --parallel "$(sysctl -n hw.ncpu)"
  cmake --install .
  cd "$PROJECT_DIR"
}

build_project(){
  mkdir -p build && cd build
  cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET" \
    -DCMAKE_PREFIX_PATH="$QT_INSTALL_DIR" \
    -DCMAKE_BUILD_TYPE=Release
  cmake --build . --parallel "$(sysctl -n hw.ncpu)"
  cmake --install .
  cd "$PROJECT_DIR"
}

# Main
check_cli
brew_deps
setup_vcpkg
build_qt
build_project

echo -e "\n🎉 Build finished!\nOpen the app with: open build/install/Inwentaryzacja.app"
