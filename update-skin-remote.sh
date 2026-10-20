#!/bin/sh
# Installed temporarily in /tmp by Update-SkinOnForce.ps1.
set -eu

target='/sdcard/Synths/Force Runtime - VST - setBfree Organ'
stage='/tmp/setbfree-skin-stage'
archive='/tmp/setbfree-skin-update.tgz'
old="$target/Plugin Skins.previous"
stopped=0

cleanup() {
  rc=$?
  if [ "$stopped" -eq 1 ]; then
    systemctl start acvs >/dev/null 2>&1 || true
  fi
  exit "$rc"
}
trap cleanup EXIT HUP INT TERM

rm -rf "$stage" "$old"
mkdir -p "$stage"
tar -xzf "$archive" -C "$stage"
test -f "$stage/Plugin Skins/TUI.json"

systemctl stop acvs
stopped=1
mv "$target/Plugin Skins" "$old"
if ! mv "$stage/Plugin Skins" "$target/Plugin Skins"; then
  mv "$old" "$target/Plugin Skins"
  exit 1
fi

rm -rf "$old" "$stage" "$archive" /tmp/update-skin-remote.sh
sync
systemctl start acvs
stopped=0
trap - EXIT HUP INT TERM
systemctl is-active --quiet acvs
