#!/usr/bin/env bash
# run-macos.sh - start a game from inside its own folder, as the cabinet does.
#
#   ./scripts/run-macos.sh <game> /path/to/the/game
#
# Starts muted; M toggles sound, or LINDBERGH_MUTE=0 starts with it on.
# Any other LINDBERGH_* switch in the environment is passed through, and
# arguments after the folder go to the game.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
die() { printf 'error: %s\n' "$*" >&2; exit 1; }

ID="${1:-}"
[ -n "$ID" ] && [ -f "$ROOT/games/$ID/game.conf" ] || die "usage: $0 <game> /path/to/the/game (games: $(ls "$ROOT/games" | tr '\n' ' '))"
. "$ROOT/games/$ID/game.conf"
BIN="$ROOT/build-mac/$ID"
[ $# -ge 2 ] || die "usage: $0 $ID \"$FOLDER_HINT\""
[ -x "$BIN" ] || die "not built yet  ->  $ROOT/scripts/build-macos.sh $ID \"$2\""

GAME="$(find_game "$2")"
[ -n "$GAME" ] || die "no $ELF in $2 (expected $TITLE's folder, $FOLDER_HINT)"

cd "$GAME/$(dirname "$ELF")"
exec "$BIN" "$(basename "$ELF")" "${@:3}"
