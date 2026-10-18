#!/usr/bin/env bash
# Docker-free build for the workspace's Debian/Ubuntu armhf cross-toolchain.
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
MPC_VST="${MPC_VST:-$here/../../working/research/mpc-vst-plugins}"
cc="${ARM_CC:-arm-linux-gnueabihf-gcc}"
strip="${ARM_STRIP:-arm-linux-gnueabihf-strip}"

command -v "$cc" >/dev/null || { echo "missing $cc" >&2; exit 1; }
mkdir -p "$here/build"

# Native artwork helper + generated params, skin and plugin-list entry.
gcc -O2 -I"$MPC_VST/tools/vendor/force-shadow/tools" \
  -o "$here/build/shadow_art" "$MPC_VST/tools/shadow_art.c" -lm
SHADOW_ART="$here/build/shadow_art" python3 "$MPC_VST/tools/gen_vst.py" "$here/vst.json"

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
"$cc" -O2 -Wall -Wextra -Wno-unused-parameter -fPIC -shared \
  -fvisibility=hidden -std=gnu11 -DLV2SYNTH -DHAVE_MEMSTREAM \
  -Ibuild -I"$MPC_VST/wrapper" -Isrc/dsp/src -Isrc/dsp/b_overdrive \
  -Isrc/dsp/b_reverb -Isrc/dsp/b_whirl \
  "${sources[@]}" -lm -Wl,--no-undefined -o build/setbfree.so
"$strip" build/setbfree.so

echo "-- exported --"
arm-linux-gnueabihf-readelf --dyn-syms -W build/setbfree.so | \
  awk '$5 == "GLOBAL" && $7 != "UND" {print $8}'
echo "-- needed --"
arm-linux-gnueabihf-readelf -d build/setbfree.so | grep NEEDED || true
echo "-- highest glibc --"
arm-linux-gnueabihf-readelf -V build/setbfree.so | \
  grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1
md5sum build/setbfree.so
