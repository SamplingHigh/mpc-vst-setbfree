#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
MPC_VST="${MPC_VST:-$here/../../working/research/mpc-vst-plugins}"
exec "$MPC_VST/tools/test_port.sh" "$here/vst.json" "$@"
