#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR=$(pwd)
VCPKG_DIR="$PROJECT_DIR/vcpkg"

QT_BIN="${QT_BIN:-/Applications/Qt/6.9.0/macos}"
QT_VERSION="6.9.0"
QT_SRC_DIR="$PROJECT_DIR/qt-src"
QT_BUILD_DIR="$PROJECT_DIR/qt-build"
QT_INSTALL_DIR="${QT_BIN:-$PROJECT_DIR/qt}"

ARCH=$(uname -m)               # arm64 or x86_64
VCPKG_TRIPLET="$( [[ $ARCH == arm64 ]] && echo arm64-osx || echo x64-osx )"

msg(){ echo "$(tput bold)▶$(tput sgr0) $*"; }
fail(){ echo "$(tput setaf 1)ERROR:$(tput sgr0) $*"; exit 1; }

check(){
  for c in git cmake ninja clang brew curl tar; do command -v $c &>/dev/null || fail "$c missing"; done
}

brew_deps(){
  msg "Installing Homebrew deps…"
  brew install --formula ninja pkg-config icu4c openssl@3 mariadb-connector-c || true
  # ensure brew’s OpenSSL dir is visible to pkg-config
  export PKG_CONFIG_PATH="$(brew --prefix)/opt/openssl@3/lib/pkgconfig:$PKG_CONFIG_PATH"
}

setup_vcpkg(){
  [[ -d $VCPKG_DIR ]] || git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
  "$VCPKG_DIR/bootstrap-vcpkg.sh" -disableMetrics
  local baseline=$(git -C "$VCPKG_DIR" rev-parse HEAD)
  sed -i '' -E "s/\"builtin-baseline\": \"[a-f0-9]+\"/\"builtin-baseline\": \"$baseline\"/" vcpkg.json || true
  "$VCPKG_DIR/vcpkg" install --triplet "$VCPKG_TRIPLET"
}

build_qt(){
  [[ -d $QT_BIN/lib ]] && { msg "Using Qt at $QT_BIN"; return; }
  msg "Building Qt $QT_VERSION from source (long)…"
  curl -LO "https://download.qt.io/archive/qt/${QT_VERSION%.*}/$QT_VERSION/single/qt-everywhere-src-$QT_VERSION.tar.xz"
  tar xf qt-everywhere-src-$QT_VERSION.tar.xz && mv qt-everywhere-src-$QT_VERSION "$QT_SRC_DIR"
  mkdir -p "$QT_BUILD_DIR" && pushd "$QT_BUILD_DIR"
  "$QT_SRC_DIR/configure" -prefix "$QT_INSTALL_DIR" -opensource -confirm-license -release \
       -nomake examples -nomake tests -skip qtquick3d -skip qtgraphs -skip qtquick3dphysics \
       -feature-sql-sqlite -feature-sql-mysql -no-warnings-are-errors
  cmake --build . --parallel "$(sysctl -n hw.ncpu)"
  cmake --install .
  popd
}

build_project(){
  cmake -S . -B build \
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake" \
        -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET" \
        -DCMAKE_PREFIX_PATH="$QT_INSTALL_DIR" \
        -DCMAKE_BUILD_TYPE=Release
  cmake --build build -j "$(sysctl -n hw.ncpu)"
  cmake --install build
}

check
brew_deps
setup_vcpkg
build_qt
build_project
msg "\n🎉 Build finished! Open with: open build/install/Inwentaryzacja.app"
