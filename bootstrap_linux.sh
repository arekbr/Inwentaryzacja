#!/usr/bin/env bash
set -euo pipefail

# ─────────────────────────── Config ──────────────────────────────
PROJECT_DIR=$(pwd)
VCPKG_DIR="$PROJECT_DIR/vcpkg"
QT_BIN="${QT_BIN:-$HOME/Qt/6.9.0/gcc_64}"  # adjust if you use another path
QT_VERSION="6.9.0"                           # used when building from source

QT_SRC_DIR="$PROJECT_DIR/qt-src"
QT_BUILD_DIR="$PROJECT_DIR/qt-build"
QT_INSTALL_DIR="${QT_BIN:-$PROJECT_DIR/qt}"

ARCH=$(uname -m)
OS=$(uname | tr '[:upper:]' '[:lower:]')
case $ARCH in
    x86_64)  VCPKG_TRIPLET="x64-$OS"   ;;
    aarch64) VCPKG_TRIPLET="arm64-$OS" ;;
    *) echo "Unsupported arch: $ARCH"; exit 1 ;;
esac

# ───────────────────────── Helper fns ────────────────────────────
msg() { echo -e "\033[1;34m▶\033[0m $*"; }
fail(){ echo -e "\033[1;31mERROR:\033[0m $*"; exit 1; }

check_reqs(){
  msg "Checking CLI tools…"
  for c in git cmake ninja g++ make curl tar; do command -v $c &>/dev/null || fail "$c missing"; done
}

install_sys_deps(){
  if command -v apt &>/dev/null; then
     msg "Installing APT dev packages…"
     sudo apt update
     sudo apt install -y build-essential ninja-build pkg-config libssl-dev zlib1g-dev \
          libmariadb-dev libsqlite3-dev libgl1-mesa-dev libicu-dev
  fi
}

setup_vcpkg(){
  if [[ ! -d $VCPKG_DIR ]]; then msg "Cloning vcpkg…"; git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"; fi
  "$VCPKG_DIR/bootstrap-vcpkg.sh" -disableMetrics
  local baseline=$(git -C "$VCPKG_DIR" rev-parse HEAD)
  sed -i -E "s/\"builtin-baseline\": \"[a-f0-9]+\"/\"builtin-baseline\": \"$baseline\"/" vcpkg.json || true
  "$VCPKG_DIR/vcpkg" install --triplet "$VCPKG_TRIPLET"
}

build_qt(){
  if [[ -d $QT_BIN/lib ]]; then msg "Using binary Qt at $QT_BIN"; return; fi
  msg "Building Qt $QT_VERSION from source (slow)…"
  [[ -d $QT_SRC_DIR ]] || {
     curl -LO "https://download.qt.io/archive/qt/${QT_VERSION%.*}/$QT_VERSION/single/qt-everywhere-src-$QT_VERSION.tar.xz"
     tar xf qt-everywhere-src-$QT_VERSION.tar.xz && mv qt-everywhere-src-$QT_VERSION "$QT_SRC_DIR"
  }
  mkdir -p "$QT_BUILD_DIR" && pushd "$QT_BUILD_DIR"
  "$QT_SRC_DIR/configure" -prefix "$QT_INSTALL_DIR" -opensource -confirm-license -release \
       -nomake examples -nomake tests -xcb -feature-sql-sqlite -feature-sql-mysql \
       -skip qtquick3d -skip qtgraphs -skip qtquick3dphysics -no-warnings-are-errors
  cmake --build . --parallel "$(nproc)"
  cmake --install .
  popd
}

build_project(){
  msg "Configuring CMake…"
  cmake -S . -B build \
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake" \
        -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET" \
        -DCMAKE_PREFIX_PATH="$QT_INSTALL_DIR" \
        -DCMAKE_BUILD_TYPE=Release
  msg "Building…" && cmake --build build -j "$(nproc)"
  msg "Installing…" && cmake --install build
}

# ─────────────────────────── Main ───────────────────────────────
check_reqs
install_sys_deps
setup_vcpkg
build_qt
build_project

msg "\n🎉  Build finished! Run with: build/install/bin/Inwentaryzacja.sh"
