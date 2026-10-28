#!/usr/bin/env bash
# run-macos.sh - start the game from inside its own folder, as the cabinet does.
#
#   ./scripts/run-macos.sh /path/to/vsg_l
#
# Starts muted; M toggles sound, or LINDBERGH_MUTE=0 starts with it on.
# Any other LINDBERGH_* switch in the environment is passed through.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/build-mac/ghostsquadevolution"

die() { printf 'error: %s\n' "$*" >&2; exit 1; }

[ $# -ge 1 ] || die "usage: $0 /path/to/vsg_l"
[ -x "$BIN" ] || die "not built yet  ->  $ROOT/scripts/build-macos.sh $1"

GAME=""
for d in "$1" "$1/vsg_l" "$1/disk1/vsg_l"; do
    [ -f "$d/vsg" ] && { GAME="$d"; break; }
done
[ -n "$GAME" ] || die "no 'vsg' in $1 (expected the game's vsg_l folder)"

cd "$GAME"
exec "$BIN" vsg "${@:2}"
