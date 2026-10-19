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
- The screenshot in `docs/images/setbfree-organ-force.png` records the running
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
