#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
set -euo pipefail

here="$(cd "$(dirname "$0")" && pwd)"
zig="${ZIG:-zig}"
command -v "$zig" >/dev/null 2>&1 || { echo "set ZIG to the Zig compiler" >&2; exit 1; }
command -v "${ARM_STRIP:-arm-linux-gnueabihf-strip}" >/dev/null 2>&1 || {
  echo "arm-linux-gnueabihf-strip is required" >&2
  exit 1
}

mkdir -p "$here/build"
wrapper="$here/build/zig-arm-gcc"
cat >"$wrapper" <<EOF
#!/usr/bin/env bash
exec "$zig" cc -target arm-linux-gnueabihf.2.31 "\$@"
EOF
chmod +x "$wrapper"

ARM_CC="$wrapper" exec "$here/build-cross.sh"
