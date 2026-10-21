#!/bin/bash
# ビルドスクリプト (対話なし)。使い方: ./build.sh [clean] [uivis]
set -e
[ -z "$G4INSTALL" ] && source "$HOME/geant4-install/bin/geant4.sh"
cd "$(dirname "$0")/.."
[ "$1" = "clean" ] && rm -rf build
mkdir -p build bin
cd build
OPT=""
if [ "$1" = "uivis" ] || [ "$2" = "uivis" ]; then OPT="-DWITH_GEANT4_UIVIS=ON"; fi
cmake $OPT ../source
make -j"$(nproc)"
make install
echo "=== ビルド完了: $(cd .. && pwd)/bin/Application_Main ==="
