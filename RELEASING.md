# Releasing

Release 0.2.0 uses a pinned revision of the public MPC VST porting kit:

- build, wrapper, skin generation, portable packaging and catalog validation:
  `SamplingHigh/mpc-vst-plugins@7f3c789494b75b8e165421809c83319cd3da6584`.

The catalog binary is built with Zig 0.17.0 for ARM hard-float and glibc 2.31:

```sh
MPC_VST=/path/to/mpc-vst-plugins-build \
ZIG=/path/to/zig-0.17.0/zig \
./build-zig.sh

MPC_VST=/path/to/mpc-vst-plugins-build ./test.sh
```

Package and validate from Linux or WSL so ZIP checksum paths use `/`:

```sh
python3 /path/to/mpc-vst-plugins-release/tools/release.py \
  --so build/setbfree.so \
  --skin "build/skin/Force Runtime - VST - setBfree Organ" \
  --entry build/pluginlist-entry.xml \
  --version 0.2.0 \
  --id setbfree-organ \
  --repo SamplingHigh/mpc-vst-setbfree \
  --license GPL-2.0-or-later \
  --about "Native tonewheel organ with three drawbar manuals, rotary speaker and keyboard splits." \
  --extra LICENSE:licenses/GPL-2.0-or-later.txt \
  --extra THIRD_PARTY_NOTICES.md:licenses/THIRD_PARTY_NOTICES.md \
  --extra SOURCE.md:licenses/SOURCE.md \
  -o dist

python3 /path/to/mpc-vst-plugins-release/tools/catalog_check.py \
  dist/setBfree-Organ-0.2.0-mpc-armv7.zip \
  --catalog --expect-id setbfree-organ \
  --expect-repo SamplingHigh/mpc-vst-setbfree --require-portable
```

The expected result is `OK: setbfree-organ 0.2.0 (0 warnings)`. The highest
required GLIBC symbol in the released binary is 2.29. The release toolkit must
come from a Linux checkout, or `tools/release/plugin_list.awk` must be normalized
to LF before packaging. The Force BusyBox `awk` rejects a CRLF copy even though
the catalog checker otherwise accepts the archive.
