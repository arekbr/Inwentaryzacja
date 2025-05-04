#!/bin/bash
set -e

# ──────────────────────────────  Konfiguracja  ──────────────────────────────── #
PROJECT_DIR="$PWD"
VCPKG_DIR="$PROJECT_DIR/vcpkg"

QT_BIN="/home/arekbr/Qt/6.8.2/gcc_64"           # jeśli ustawione → używamy binarnego Qt
QT_VERSION="6.8.2"

QT_SRC_DIR="$PROJECT_DIR/qt-src"
QT_BUILD_DIR="$PROJECT_DIR/qt-build"
QT_INSTALL_DIR="${QT_BIN:-$PROJECT_DIR/qt}"

ARCH=$(uname -m)
OS=$(uname | tr '[:upper:]' '[:lower:]')
case "$ARCH" in
  x86_64)  VCPKG_TRIPLET="x64-$OS"   ;;
  aarch64) VCPKG_TRIPLET="arm64-$OS" ;;
  armv7l)  VCPKG_TRIPLET="arm-$OS"   ;;
  *) echo "Unsupported arch: $ARCH"; exit 1 ;;
esac

# ─────────────────────  Funkcje pomocnicze  ─────────────────────────────────── #
fail() { echo "ERROR: $1"; exit 1; }

check_requirements() {
  echo "▶ Checking required CLI tools..."
  for cmd in git cmake ninja g++ make curl tar; do
    command -v "$cmd" >/dev/null || fail "$cmd not found – install it first"
  done
  echo "✔ All required tools found"
}

install_system_dependencies() {
  echo "▶ Installing system‑wide dev packages (Ubuntu/Deb‑family)..."
  sudo apt update
  sudo apt install -y \
      build-essential ninja-build pkg-config \
      libgl1-mesa-dev libglu1-mesa-dev libfontconfig1-dev \
      libfreetype6-dev libx11-dev libxext-dev libxfixes-dev libxi-dev \
      libxrender-dev libxkbcommon-dev libxkbcommon-x11-dev libxcb1-dev \
      libxcb-util-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-shm0-dev \
      libxcb-sync-dev libxcb-randr0-dev libxcb-image0-dev libxcb-keysyms1-dev \
      libxcb-icccm4-dev libxcb-xinerama0-dev libxcb-cursor-dev libssl-dev \
      zlib1g-dev libpng-dev libjpeg-dev libmariadb-dev
  echo "✔ System dependencies installed"
}

setup_vcpkg() {
  if [ ! -d "$VCPKG_DIR" ]; then
    echo "▶ Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
  else
    echo "▶ Updating vcpkg..."
    git -C "$VCPKG_DIR" pull
  fi

  echo "▶ Bootstrapping vcpkg..."
  "$VCPKG_DIR/bootstrap-vcpkg.sh"

  # zawsze aktualizuj builtin‑baseline
  BASELINE=$(git -C "$VCPKG_DIR" rev-parse HEAD)
  echo "    Using builtin-baseline: $BASELINE"
  if grep -q '"builtin-baseline":' vcpkg.json; then
    sed -i "s/\"builtin-baseline\": \".*\"/\"builtin-baseline\": \"$BASELINE\"/" vcpkg.json
  else
    sed -i "/\"version-string\"/a \  \"builtin-baseline\": \"$BASELINE\"," vcpkg.json
  fi

  echo "▶ Installing ports with vcpkg (manifest mode)..."
  "$VCPKG_DIR/vcpkg" install --triplet "$VCPKG_TRIPLET"
}

build_qt_from_source() {
  if [ -n "$QT_BIN" ]; then
    echo "▶ Skipping Qt build – using binary Qt at $QT_BIN"
    return
  fi

  if [ ! -d "$QT_SRC_DIR" ]; then
    echo "▶ Downloading Qt $QT_VERSION sources..."
    curl -L "https://download.qt.io/archive/qt/${QT_VERSION%.*}/${QT_VERSION}/single/qt-everywhere-src-${QT_VERSION}.tar.xz" -o qt.tar.xz
    tar -xf qt.tar.xz
    mv "qt-everywhere-src-${QT_VERSION}" "$QT_SRC_DIR"
    rm qt.tar.xz
  fi

  echo "▶ Configuring Qt..."
  mkdir -p "$QT_BUILD_DIR"
  cd "$QT_BUILD_DIR"
  "$QT_SRC_DIR/configure" \
      -prefix "$QT_INSTALL_DIR" \
      -opensource -confirm-license -nomake examples -nomake tests \
      -release -xcb -feature-xcb -feature-xkbcommon -feature-opengl \
      -feature-sql-mysql \ -skip qtgraphs \ -skip qtquick3dphysics \ -skip qtquick3d \ -no-warnings-are-errors

  grep -q "xcb.*yes" config.summary || fail "Qt configured WITHOUT XCB – check X11/XCB dev packages"

  echo "▶ Building Qt..."
  cmake --build . --parallel "$(nproc)"
  echo "▶ Installing Qt..."
  cmake --install .
  cd "$PROJECT_DIR"

  [ -f "$QT_INSTALL_DIR/plugins/platforms/libqxcb.so" ] \
      || fail "libqxcb.so missing after build – aborting"
}

build_project() {
  echo "▶ Configuring CMake project..."
  mkdir -p build && cd build
  cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET" \
    -DCMAKE_PREFIX_PATH="$QT_INSTALL_DIR" \
    -DCMAKE_BUILD_TYPE=Release

  echo "▶ Building project..."
  cmake --build . --parallel "$(nproc)"
  echo "▶ Installing project..."
  cmake --install .
  cd "$PROJECT_DIR"
}

# ─────────────────────────────  Main  ───────────────────────────────────────── #
check_requirements
install_system_dependencies
setup_vcpkg
build_qt_from_source
build_project

echo -e "\n🎉  Build finished!"
echo "Run the application with:"
echo "  ./build/install/bin/Inwentaryzacja.sh"
