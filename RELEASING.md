# Releasing

Release 0.3.1 is built and packaged with the Open MPC porting kit pinned to:

`open-mpc/mpc-vst-plugins@0dc17120cd7e952e95eeecdd820ac9e495a75ee9`

The same revision is present in both `uses:` and `tools_ref:` in
`.github/workflows/release.yml`. The reusable workflow builds the ARMv7 plugin,
runs the host test, validates the package and creates a draft GitHub release.
It also produces the MPC OS 3.10.1 SDK-format folder zip.

## GitHub Actions release

Run the **Release VST** workflow with version `0.3.1`. A dry run builds the
artifacts without creating a release; a normal run creates or updates draft tag
`v0.3.1`.

Before publishing the draft:

1. Inspect every generated skin preview.
2. Install the exact ARMv7 artifact on a Force and test audio, every page,
   touchscreen control, Q-Links, Data Wheel, project save and project reload.
3. Confirm `tested.json` records the tested device and firmware.
4. Publish the draft release. Publishing creates tag `v0.3.1`.

The release and catalog target ARMv7 first-generation devices only. Do not
publish or advertise a Gen2 build without verification on physical hardware.

## Reproduce locally

From Linux or WSL with Zig 0.17.0 and ARM binutils installed:

```sh
MPC_VST=/path/to/mpc-vst-plugins \
ZIG=/path/to/zig \
./build-zig.sh

MPC_VST=/path/to/mpc-vst-plugins ./test.sh
```

The ARMv7 binary's highest required GLIBC symbol must remain no newer than the
catalog rules allow. `params.lock` must remain unchanged unless a deliberately
incompatible project-compatibility release is being made.
