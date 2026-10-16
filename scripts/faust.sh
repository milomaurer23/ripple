#!/usr/bin/env bash
# Regenerates plugin/Source/RippleDSP.h from plugin/Faust/Ripple.dsp. The generated header is
# committed, so building the plugin does not need Faust installed; only editing the .dsp does.
#   scripts/faust.sh          regenerate
#   scripts/faust.sh --check  fail if the committed header is stale (used by pluginval.sh)
set -euo pipefail
cd "$(dirname "$0")/.."

DSP=plugin/Faust/Ripple.dsp
ARCH=plugin/Faust/ripple-arch.cpp
OUT=plugin/Source/RippleDSP.h

if ! command -v faust >/dev/null 2>&1; then
    if [[ "${1:-}" == "--check" ]]; then
        echo "faust not installed: skipping the RippleDSP.h freshness check"
        exit 0
    fi
    echo "faust not found. Install it (brew install faust) and re-run." >&2
    exit 1
fi

# -double: the clock and gain run in double precision, like the C++ engine they replace.
# -inpl:   processBlock hands Faust the same buffers for input and output.
generate() { faust -double -inpl -lang cpp -cn RippleDSP -i -a "$ARCH" "$DSP" -o "$1"; }

if [[ "${1:-}" == "--check" ]]; then
    # Different Faust versions emit different code for the same .dsp, so only compare like
    # with like.
    have="$(faust --version | sed -n "1s/.*Version //p")"
    built="$(sed -n 's/^Code generated with Faust \([^ ]*\).*/\1/p' "$OUT")"
    if [[ "$have" != "$built" ]]; then
        echo "$OUT was generated with Faust $built, installed is $have: skipping the freshness check"
        exit 0
    fi
    tmp="$(mktemp)"
    trap 'rm -f "$tmp"' EXIT
    generate "$tmp"
    if ! diff -q -I '^Code generated with Faust' -I '^Compilation options' "$tmp" "$OUT" >/dev/null; then
        echo "$OUT is out of date with $DSP. Run scripts/faust.sh." >&2
        exit 1
    fi
    echo "$OUT is up to date"
else
    generate "$OUT"
    echo "wrote $OUT ($(faust --version | sed -n 1p))"
fi
