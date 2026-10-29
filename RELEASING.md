# Releasing

Release 0.3.0 uses pinned revisions of the public MPC VST porting kit:

- build, wrapper and skin generation:
  `SamplingHigh/mpc-vst-plugins@cf1a5609ed706fc966f653bb144e979d07c67b74`;
- portable packaging and catalog validation:
  `SamplingHigh/mpc-vst-plugins@593324542eaeeaaa0d9ca9cc43ba7871cd1f8732`.

The Gen1/Force binary is built with Zig 0.17.0 for ARM hard-float and glibc
2.31. The experimental Gen2 binary uses the same sources and wrapper, targeting
aarch64 and glibc 2.31:

```sh
MPC_VST=/path/to/mpc-vst-plugins-build \
ZIG=/path/to/zig-0.17.0/zig \
./build-zig.sh

MPC_VST=/path/to/mpc-vst-plugins-build ./test.sh

MPC_VST=/path/to/mpc-vst-plugins-build \
ZIG=/path/to/zig-0.17.0/zig \
./build-zig-aarch64.sh

MPC_VST=/path/to/mpc-vst-plugins-build ./test.sh aarch64
```

Package and validate from Linux or WSL so ZIP checksum paths use `/`:

```sh
python3 /path/to/mpc-vst-plugins-release/tools/release.py \
  --so build/setbfree.so \
  --skin "build/skin/Force Runtime - VST - setBfree Organ" \
  --entry build/pluginlist-entry.xml \
  --version 0.3.0 \
  --id setbfree-organ \
  --repo SamplingHigh/mpc-vst-setbfree \
  --license GPL-2.0-or-later \
  --bench build/bench.txt \
  --about "Native tonewheel organ with three drawbar manuals, rotary speaker and keyboard splits." \
  --extra LICENSE:licenses/GPL-2.0-or-later.txt \
  --extra THIRD_PARTY_NOTICES.md:licenses/THIRD_PARTY_NOTICES.md \
  --extra SOURCE.md:licenses/SOURCE.md \
  -o dist

python3 /path/to/mpc-vst-plugins-release/tools/release.py \
  --so build/aarch64/setbfree.so \
  --skin "build/skin/Force Runtime - VST - setBfree Organ" \
  --entry build/pluginlist-entry.xml \
  --version 0.3.0 \
  --id setbfree-organ \
  --repo SamplingHigh/mpc-vst-setbfree \
  --license GPL-2.0-or-later \
  --about "Native tonewheel organ with three drawbar manuals, rotary speaker and keyboard splits." \
  --extra LICENSE:licenses/GPL-2.0-or-later.txt \
  --extra THIRD_PARTY_NOTICES.md:licenses/THIRD_PARTY_NOTICES.md \
  --extra SOURCE.md:licenses/SOURCE.md \
  -o dist

python3 /path/to/mpc-vst-plugins-release/tools/catalog_check.py \
  dist/setBfree-Organ-0.3.0-mpc-armv7.zip \
  --catalog --expect-id setbfree-organ \
  --expect-repo SamplingHigh/mpc-vst-setbfree --require-portable

python3 /path/to/mpc-vst-plugins-release/tools/catalog_check.py \
  dist/setBfree-Organ-0.3.0-mpc-aarch64.zip \
  --catalog --expect-id setbfree-organ \
  --expect-repo SamplingHigh/mpc-vst-setbfree --require-portable
```

Both checks must report `OK: setbfree-organ 0.3.0 (0 warnings)`. The highest
required GLIBC symbol in both released binaries is 2.29. The release toolkit must
come from a Linux checkout, or `tools/release/plugin_list.awk` must be normalized
to LF before packaging. The Force BusyBox `awk` rejects a CRLF copy even though
the catalog checker otherwise accepts the archive.

The generated aarch64 ZIP is copied to
`setBfree-Organ-0.3.0-gen2-experimental.zip` and published on the project's
GitHub Release as **experimental, hardware untested**. The intentionally
non-standard asset name prevents the existing catalog entry from discovering
it as an `*-mpc-aarch64.zip` sibling. Do not publish the standard filename or
claim Gen2 verification until installation, audio, touchscreen, Q-Link, preset
and project recall have been checked on physical Gen2 hardware.
