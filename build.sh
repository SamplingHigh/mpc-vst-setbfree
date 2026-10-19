#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
MPC_VST="${MPC_VST:-$here/../../working/research/mpc-vst-plugins}"
[ -f "$MPC_VST/tools/build_port.sh" ] || { echo "set MPC_VST to mpc-vst-plugins" >&2; exit 1; }
exec "$MPC_VST/tools/build_port.sh" "$here/vst.json"
