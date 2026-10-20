#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
# Regenerate only the MPC skin.  The ARM plugin binary is left untouched.
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
if [ -z "${MPC_VST:-}" ]; then
  for candidate in "$here/../mpc-vst-plugins" "$here/../../working/research/mpc-vst-plugins-current"; do
    if [ -f "$candidate/tools/gen_vst.py" ]; then
      MPC_VST="$candidate"
      break
    fi
  done
fi
[ -n "${MPC_VST:-}" ] || {
  echo "Set MPC_VST to the mpc-vst-plugins checkout." >&2
  exit 1
}

if [ -z "${HTML_ART_PY:-}" ]; then
  if [ -x "$HOME/force-runtime/.venv-html-art/bin/python3" ]; then
    HTML_ART_PY="$HOME/force-runtime/.venv-html-art/bin/python3"
  else
    HTML_ART_PY=python3
  fi
fi

mkdir -p "$here/build"
cat >"$here/build/html_art_runner" <<EOF
#!/usr/bin/env bash
exec "$HTML_ART_PY" "$MPC_VST/tools/html_art.py" "\$@"
EOF
chmod +x "$here/build/html_art_runner"

SHADOW_ART="$here/build/html_art_runner" \
  python3 "$MPC_VST/tools/gen_vst.py" "$here/vst.json"

echo "Skin-only build complete; the plugin binary was not rebuilt."
