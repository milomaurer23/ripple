#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
scripts/faust.sh --check
cmake --build build --config Release --target RippleFaustTests
"$(find -L build -type f -perm -u+x -name RippleFaustTests -path '*Release*' | head -n 1)"
cmake --build build --config Release --target Ripple_VST3 Ripple_AU
cmake --build build --config Release --target validate
