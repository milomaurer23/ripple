#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cmake --build build --config Release --target Ripple_VST3 Ripple_AU
cmake --build build --config Release --target validate
