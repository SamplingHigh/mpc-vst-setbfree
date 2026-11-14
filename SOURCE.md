# Source for the binary release

The complete corresponding source for release 0.3.1 is the `v0.3.1` tag:

<https://github.com/SamplingHigh/mpc-vst-setbfree/tree/v0.3.1>

The generic MPC VST2 wrapper compiled into the plugin and the skin-generation
tools are in the public Open MPC `mpc-vst-plugins` repository. The build and
release workflow are pinned to commit
`0dc17120cd7e952e95eeecdd820ac9e495a75ee9`:

<https://github.com/open-mpc/mpc-vst-plugins/tree/0dc17120cd7e952e95eeecdd820ac9e495a75ee9>

The ARMv7 and experimental aarch64 builds use the same setBfree sources, plugin
UID, parameter indices and state format. Reproduction commands are in
`RELEASING.md`.

The port and combined binary are licensed under GPL-2.0-or-later. See
`LICENSE`, `THIRD_PARTY_NOTICES.md`, and `src/VENDORED.md`.
