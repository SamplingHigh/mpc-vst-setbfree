# setBfree Organ for MPC OS

A native VST2 port of the [setBfree](https://github.com/pantherb/setBfree)
tonewheel organ engine for Akai MPC standalone devices and Force. It runs in
MPC's plugin host and has a native touchscreen skin. The ARMv7 release is
verified on Force; the aarch64 Gen2 build is experimental and has not yet been
run on Gen2 hardware.

![setBfree Organ running on Akai Force](docs/images/setbfree-organ.png)

The current release is **0.3.0**. It provides:

- upper and lower drawbars with traditional colour groups, plus 16', 8' and 4'
  pedal controls;
- touchscreen, Data Wheel and Q-Link control, including MPC's page-following
  KNOBS screen;
- a five-page UPPER, LOWER/PEDAL, ORGAN, FX and SETTINGS touchscreen layout;
- percussion, per-manual vibrato/chorus, scanner, swell and manual-level
  controls;
- editable overdrive, reverb, Leslie motor, key-click and leakage parameters;
- switchable sustain-pedal or Leslie-speed pedal operation;
- one-keyboard operation with optional upper/lower or upper/lower/pedal splits;
- VST chunk state for project save and reload.

## Screens

![Upper drawbars and performance controls](docs/images/setbfree-organ-01-upper.png)

![Lower and pedal drawbars](docs/images/setbfree-organ-02-lower_pedal.png)

![Percussion, scanner and manual controls](docs/images/setbfree-organ-03-organ.png)

![Overdrive and reverb controls](docs/images/setbfree-organ-04-fx.png)

![Leslie, key-click and leakage settings](docs/images/setbfree-organ-05-settings.png)

Every incoming channel-voice message is routed to setBfree's upper-manual MIDI
channel. With split mode disabled, the whole keyboard drives the upper manual.
The two split modes use setBfree's native channel-A key map. MIDI note ranges in
the engine are upper/lower C2-C7 (36-96) and pedal C1-C3 (24-49).

The normal audio path measured 8.9% p99 and 12.4% worst-block CPU at 16 voices
on a Force. Changing one of the four leakage/crosstalk settings rebuilds the
organ core and can briefly mute audio; these controls are therefore on the
SETTINGS page and are intended for sound setup rather than performance moves.

## Install

Download the package for the device from the GitHub release and follow its
included `INSTALL.md`:

- `setBfree-Organ-0.3.0-mpc-armv7.zip`: Force and first-generation MPCs;
- `setBfree-Organ-0.3.0-gen2-experimental.zip`: Gen2 MPCs, experimental and
  hardware untested. Its intentionally non-catalog filename prevents the public
  catalog from advertising it before a physical-device confirmation.

Installation requires root SSH access and restarts the MPC application. The
two packages use the same plugin UID, parameter indices and state format, so
projects remain compatible across architectures. Gen2 needs a writable
removable-media `Synths` folder passed to the installer with `-t`; its internal
`/synths/Synths` mount is read-only on the hardware tested by the porting kit.

## Build and test

The build uses the public
[mpc-vst-plugins](https://github.com/SamplingHigh/mpc-vst-plugins) porting kit. Clone
both repositories, check out the tested porting-kit revision, then point
`MPC_VST` at the kit:

```sh
git clone https://github.com/SamplingHigh/mpc-vst-plugins.git
git -C mpc-vst-plugins checkout cf1a5609ed706fc966f653bb144e979d07c67b74
git clone https://github.com/SamplingHigh/mpc-vst-setbfree.git
cd mpc-vst-setbfree
MPC_VST=../mpc-vst-plugins ./test.sh
MPC_VST=../mpc-vst-plugins ./test.sh aarch64
MPC_VST=../mpc-vst-plugins ./build.sh all
```

`build.sh` needs Docker with ARMv7 and ARM64/QEMU support. `build.sh armv7` or
`build.sh aarch64` selects one target; the default builds both. If an
`arm-linux-gnueabihf-gcc` toolchain is installed, use `build-cross.sh` instead.
HTML skin artwork also needs Python, Pillow and Playwright Chromium; set
`HTML_ART_PY` when that Python interpreter is outside the default path.

For a catalog-compatible build without Docker, install Zig and run:

```sh
ZIG=/path/to/zig ./build-zig.sh
MPC_VST=../mpc-vst-plugins ZIG=/path/to/zig ./build-zig-aarch64.sh
```

For layout and artwork changes, `build-skin.sh` regenerates only the skin. On
Windows, `Update-SkinOnForce.bat` builds that skin, uploads only `Plugin Skins`
and restarts the MPC application without replacing the plugin binary. Set
`FORCE_HOST`, pass `-ForceHost` to the PowerShell script, or put the device
address in a local `.force-host` file.

Both 0.3.0 binaries have a highest required GLIBC symbol of 2.29. The ARMv7
build targets ARM hard-float with glibc 2.31 compatibility. The aarch64 build
targets Gen2 MPC OS 3.x; it passes the ARM64 functional host test and catalog
package validation, but is not marked as hardware verified and is deliberately
not submitted to the public plugin catalog yet.

## Source and licence

This complete port is distributed under **GPL-2.0-or-later**. The full licence
text is in [LICENSE](LICENSE).

The setBfree engine source is vendored under `src/dsp` from upstream commit
[`2f41983b5b535bda9b1b1db38a06da29d50ff557`](https://github.com/pantherb/setBfree/tree/2f41983b5b535bda9b1b1db38a06da29d50ff557).
Its original copyright and GPL notices are preserved. The MPC host adapter,
parameter definitions, skin sources and build scripts are included in this
repository, so the published source corresponds to the released binary.

See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) and
[`src/VENDORED.md`](src/VENDORED.md) for provenance and the exact vendoring
changes. No Akai plugin code or artwork is included. Akai and MPC are trademarks
of their respective owners; this project is independent and unofficial.
