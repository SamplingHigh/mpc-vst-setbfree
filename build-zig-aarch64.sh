#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
# Docker-free Gen2 build with Zig. The architecture-independent skin and
# generated params must already exist (run build-skin.sh after layout changes).
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
MPC_VST="${MPC_VST:-$here/../../working/research/mpc-vst-plugins}"
zig="${ZIG:-zig}"
command -v "$zig" >/dev/null 2>&1 || { echo "set ZIG to the Zig compiler" >&2; exit 1; }
[ -f "$MPC_VST/wrapper/vst2_wrap.c" ] || { echo "set MPC_VST to mpc-vst-plugins" >&2; exit 1; }
[ -f "$here/build/params.h" ] || { echo "run build-skin.sh first to generate build/params.h" >&2; exit 1; }

mkdir -p "$here/build/aarch64"
cd "$here"
sources=(
  src/setbfree_engine.c
  src/dsp/src/midi.c src/dsp/src/cfgParser.c src/dsp/src/program.c
  src/dsp/src/vibrato.c src/dsp/src/state.c src/dsp/src/tonegen.c
  src/dsp/src/pgmParser.c src/dsp/src/memstream.c src/dsp/src/midnam.c
  src/dsp/b_whirl/eqcomp.c src/dsp/b_whirl/whirl.c
  src/dsp/b_overdrive/overdrive.c src/dsp/b_reverb/reverb.c
  "$MPC_VST/wrapper/vst2_wrap.c"
)
"$zig" cc -target aarch64-linux-gnu.2.31 \
  -O2 -Wall -Wextra -Wno-unused-parameter -fPIC -shared \
  -fvisibility=hidden -std=gnu11 -DLV2SYNTH -DHAVE_MEMSTREAM \
  -Ibuild -I"$MPC_VST/wrapper" -Isrc/dsp/src -Isrc/dsp/b_overdrive \
  -Isrc/dsp/b_reverb -Isrc/dsp/b_whirl \
  "${sources[@]}" -lm -Wl,--no-undefined -o build/aarch64/setbfree.so

file build/aarch64/setbfree.so
readelf -d build/aarch64/setbfree.so | grep NEEDED || true
echo "highest glibc: $(readelf -V build/aarch64/setbfree.so | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1)"
sha256sum build/aarch64/setbfree.so
