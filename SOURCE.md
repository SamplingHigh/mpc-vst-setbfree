# Source for the binary release

The complete corresponding source for release 0.1.12 is the `v0.1.12` tag:

<https://github.com/SamplingHigh/mpc-vst-setbfree/tree/v0.1.12>

The generic MPC VST2 wrapper compiled into the plugin and the skin-generation
tools are in the public `mpc-vst-plugins` fork. The build uses branch
`setbfree-drawbar-support`, commit
`66eca176028b5fd63622dd7769451d5e28eb11a1`:

<https://github.com/SamplingHigh/mpc-vst-plugins/tree/66eca176028b5fd63622dd7769451d5e28eb11a1>

The portable release package is generated with `release.py` from upstream
tooling commit `5933245d`.

The port and combined binary are licensed under GPL-2.0-or-later. See
`LICENSE`, `THIRD_PARTY_NOTICES.md`, and `src/VENDORED.md`.
