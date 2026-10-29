# Source for the binary release

The complete corresponding source for release 0.3.0 is the `v0.3.0` tag:

<https://github.com/SamplingHigh/mpc-vst-setbfree/tree/v0.3.0>

The generic MPC VST2 wrapper compiled into the plugin and the skin-generation
tools are in the public `mpc-vst-plugins` fork. The build uses branch
`feature/skin-studio-force-controls`, commit
`cf1a5609ed706fc966f653bb144e979d07c67b74`:

<https://github.com/SamplingHigh/mpc-vst-plugins/tree/cf1a5609ed706fc966f653bb144e979d07c67b74>

The portable release package is generated with `release.py` from upstream
tooling commit `593324542eaeeaaa0d9ca9cc43ba7871cd1f8732`.

The ARMv7 and aarch64 assets use the same setBfree and wrapper sources, plugin
UID, parameter indices and state format. Only the compiler target and packaged
ELF binary differ. Reproduction commands for both assets are in
`RELEASING.md`.

The port and combined binary are licensed under GPL-2.0-or-later. See
`LICENSE`, `THIRD_PARTY_NOTICES.md`, and `src/VENDORED.md`.
