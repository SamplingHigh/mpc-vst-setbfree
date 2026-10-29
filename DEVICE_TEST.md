# Device test log

## 2026-10-18 — Akai Force, MPC OS armv7

- Installed `setBfree Organ 0.1.0` at `/sdcard/vst/setbfree.so`.
- Installed the skin at `/sdcard/Synths/Force Runtime - VST - setBfree Organ`.
- Registered one plugin-list entry with UID `53624631` (`SbF1`).
- Installed binary MD5: `cc64d79ade05083355f1cc7264701ec5`.
- MPC restarted successfully after installation.
- Settings backup created at
  `/media/az01-internal/Settings/MPC/MPC.settings.bak-setbfree-20261018-230037`.

The remaining device check is the interactive play test: instantiate the plugin,
play all three split modes, exercise the skin/Q-Links, and save/reload a project.

## 2026-10-19 — Akai Force, MPC OS 3.9.1.2

- Installed and interactively tested release 0.1.11.
- Confirmed drawbar control from the touchscreen, focused Data Wheel and Q-Links.
- Confirmed the displayed 0–8 values and the audio gain direction: 0 is silent,
  8 is full level.
- Confirmed the default drawbar registrations, three drawbar colour groups and
  overdrive defaults on the device.
- The updated screenshot in `docs/images/setbfree-organ-01-upper.png` records the running
  plugin on the tested Force.

## 2026-10-19 — release 0.1.12 portable package

- Installed the final portable package on the same Force running MPC OS 3.9.1.2.
- The installer migrated the legacy `/sdcard/vst/setbfree.so` installation and
  registered exactly one portable plugin-list entry.
- Installed binary MD5: `7667000d4a7f1ba8c8525445567b5850`.
- Confirmed manifest version 0.1.12, active MPC service and a running MPC process.
- Instantiated setBfree Organ through MPC Remote. The upper drawbars opened at
  `888000000`, and the overdrive controls opened at `47 / 16 / 47`.
- Confirmed from `/proc/<MPC pid>/maps` that MPC loaded
  `/sdcard/Synths/Force Runtime - VST - setBfree Organ/setbfree.so`.

The first installation attempt exposed a CRLF copy of `plugin_list.awk` that the
device BusyBox `awk` rejected. The final release package contains the same file
with LF line endings; the installer then completed, removed the legacy binary,
and restarted MPC successfully.

## 2026-10-19 — 0.1.13 advanced controls developer build

- The host test passed with 55 parameters, chunk-state restore, NRPN control and
  3000 concurrent screen-side parameter changes while rendering.
- Installed the portable 0.1.13 package on the Force and restarted MPC.
- Confirmed manifest version 0.1.13, one plugin-list entry, an active MPC service
  and a running MPC process.
- Installed binary MD5: `4c08143b3db3c8041ae32eec2e309898`.
- The MPC Remote HTTP endpoint did not answer after this restart, so the new
  ORGAN page still needs a visual and listening check on the device.

## 2026-10-19 — 0.1.14 skin readability update

- Regenerated the skin with 30 px knob/drawbar names, 37 px values, and 24 px
  selector text; updated the page background to `#2B241E`.
- Catalog package validation passed with 0 warnings.
- Installed the package on the Force; the installer completed, backed up
  `MPC.settings`, and restarted MPC.
- Confirmed one plugin-list entry, an active `acvs` service and MPC process,
  and a matching installed `TUI.json` checksum (`2c69d33b42b8becc769055c9fb25a95a`).

## 2026-10-20 — release 0.2.0

- Built and validated the portable catalog package with 0 warnings; the host
  test suite passed with 71 parameters.
- Installed the final package on the Force and restarted MPC successfully.
- Confirmed manifest version 0.2.0, exactly one plugin-list entry, an active
  `acvs` service and a running MPC process (PID 648 after the restart).
- Installed binary MD5: `16cb1529a8530261b93fee55fc1de857`.
- Confirmed the four-page UPPER, LOWER/PEDAL, ORGAN and SETTINGS layout from the
  device screenshots in `docs/images/`.
- The release packager now normalizes `plugin_list.awk` to LF line endings and
  has regression coverage for Windows checkouts; all 130 toolkit tests passed.

## 2026-10-22 — release 0.2.1

- Rebuilt the plugin with the wrapper fix that stops ordinary MPC parameter
  writes from requesting an `UpdateDisplay` and rate-limits required display
  refreshes to one per 50 ms.
- The host regression test issued 32 writes to `pedal_split` over 4096 frames
  and observed zero `UpdateDisplay` calls; the complete 71-parameter host test
  passed.
- Added and validated `params.lock` so future builds reject parameter-index
  changes that would break projects, automation or Q-Link maps.
- Installed the final portable package on the Force and restarted MPC. Confirmed
  manifest version 0.2.1, exactly one plugin-list entry, an active `acvs` service
  and a running MPC process (PID 626 after the restart).
- Installed binary MD5: `6794d55636bfd3e1237f80109d06e1e3`.

The remaining interactive check is moving a physical Q-Link on track edit →
automation; this input path cannot be exercised over SSH.

## 2026-10-24 — release 0.3.0

- Built the complete 80-parameter ARMv7 plugin and five-page skin from the
  public pinned toolkit revision; the host test passed, including state restore,
  Q-Link/Data Wheel stepping and concurrent parameter changes while rendering.
- The Force CPU bench measured 8.9% p99 and 12.4% worst-block use at 16 voices.
  Its general Q-Link sweep intentionally reports FAIL because it reaches the
  four SETTINGS-page crosstalk controls, each of which rebuilds the organ core.
- The first portable-package install caught CRLF line endings in the packaged
  BusyBox `awk` helper before replacing the existing plugin. The packager was
  fixed, its regression tests passed, and the rebuilt package validated with
  zero catalog warnings.
- Installed the corrected 0.3.0 portable package on the Force. Confirmed one
  plugin-list entry, active `acvs`, a running MPC process (PID 6211), and an
  installed binary MD5 of `9f9d1140ea1c441897b16418f4da0ba5`.

## 2026-10-29 — experimental Gen2/aarch64 build

- Built the unchanged 0.3.0 sources and pinned wrapper as an ELF64 aarch64 VST2
  library. It exports `VSTPluginMain`, needs only `libm`, `libc` and
  `libpthread`, and its highest required GLIBC symbol is 2.29.
- Ran the complete 80-parameter functional host test as a static aarch64
  executable under QEMU. Audio generation, two instances, parameter writes,
  Data Wheel stepping, chunk restore, CC/NRPN control and concurrent
  screen-side changes all passed.
- Packaged `setBfree-Organ-0.3.0-mpc-aarch64.zip`; catalog validation reported
  `OK: setbfree-organ 0.3.0 (0 warnings)`.
- No physical Gen2 device was available. Installation, MPC loading, audio,
  touchscreen rendering, Q-Links and project recall remain hardware untested;
  the asset is therefore published only on the project release under the
  non-catalog filename `setBfree-Organ-0.3.0-gen2-experimental.zip` and is
  withheld from the public plugin catalog.
