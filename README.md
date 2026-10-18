# setBfree Organ for MPC OS

Early MPC/Force VST2 port of the [setBfree](https://github.com/pantherb/setBfree)
tonewheel organ engine. The first milestone runs the engine in-process and exposes:

- upper, lower and pedal drawbars;
- percussion, vibrato/chorus, swell, overdrive, reverb and rotary speed;
- one-keyboard operation with optional upper/lower or upper/lower/pedal splits;
- VST chunk state for project save and reload.

The MPC skin uses vertical, pull-down drawbars and 1.7x native label/value
fonts. Its artwork is generated from `skin.css`; no third-party organ artwork
is included.

Every incoming channel-voice message is routed to setBfree's upper-manual MIDI
channel. With split mode disabled the whole keyboard drives the upper manual.
The two split modes use setBfree's native channel-A key map. MIDI note ranges in
the engine are upper/lower C2-C7 (36-96) and pedal C1-C3 (24-49).

Build from WSL or Linux with Docker/QEMU available:

```sh
./test.sh
./build.sh
```

When Docker is unavailable but an `arm-linux-gnueabihf-gcc` toolchain is
installed, use `./build-cross.sh` for the ARM package build. Browser artwork
also needs Python with Pillow and Playwright Chromium; set `HTML_ART_PY` to
that Python interpreter when it is not available on the default path.

`MPC_VST` may be set to a different `mpc-vst-plugins` checkout. The default is
the sibling workspace checkout at `../../working/research/mpc-vst-plugins`.

This project is GPL-2.0-or-later because it links the setBfree engine. See
`LICENSE` and `src/VENDORED.md`.
