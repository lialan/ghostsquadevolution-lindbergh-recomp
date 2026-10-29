#!/usr/bin/env bash
# run-macos.sh - start the game from inside its own folder, as the cabinet does.
#
#   ./scripts/run-macos.sh "/path/to/House of the Dead 4"
#
# Starts muted; M toggles sound, or LINDBERGH_MUTE=0 starts with it on.
# Any other LINDBERGH_* switch in the environment is passed through.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/build-mac/hotd4"

die() { printf 'error: %s\n' "$*" >&2; exit 1; }

[ $# -ge 1 ] || die "usage: $0 "/path/to/House of the Dead 4""
[ -x "$BIN" ] || die "not built yet  ->  $ROOT/scripts/build-macos.sh $1"

GAME=""
for d in "$1" "$1/House of the Dead 4"; do
    [ -f "$d/elf/hod4M.elf" ] && { GAME="$(cd "$d/elf" && pwd)"; break; }
done
[ -n "$GAME" ] || die "no elf/hod4M.elf in $1"

cd "$GAME"
exec "$BIN" hod4M.elf "${@:2}"
