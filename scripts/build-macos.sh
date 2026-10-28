#!/usr/bin/env bash
# build-macos.sh - lift your own copy of the game and build it for macOS.
#
#   ./scripts/build-macos.sh /path/to/vsg_l
#
# The folder is the game's own: it holds `vsg` (the ELF, no extension),
# `snd_util.so` and `librnalindbergh_jr.so`. A disc root holding vsg_l/ or
# disk1/vsg_l/ is fine too. Nothing from it is copied into this repo; the
# lifted C goes to the gitignored games/ghostsquadevolution/generated/.
#
# Safe to run again: unchanged lifted files keep their timestamps, so a
# rebuild only recompiles what moved.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TOOLKIT="$ROOT/lindberghrecomp"
GEN="$ROOT/games/ghostsquadevolution/generated"
BUILD="$ROOT/build-mac"
VENV="$ROOT/.venv"

die() { printf 'error: %s\n' "$*" >&2; exit 1; }
say() { printf '==> %s\n' "$*"; }

[ "$(uname -s)" = Darwin ] || die "this script is for macOS; see the README for Windows"
[ $# -ge 1 ] || die "usage: $0 /path/to/vsg_l"

# ---- the game folder ----
GAME=""
for d in "$1" "$1/vsg_l" "$1/disk1/vsg_l"; do
    [ -f "$d/vsg" ] && { GAME="$(cd "$d" && pwd)"; break; }
done
[ -n "$GAME" ] || die "no 'vsg' in $1 (expected the game's vsg_l folder)"
for f in vsg snd_util.so librnalindbergh_jr.so; do
    [ -f "$GAME/$f" ] || die "$GAME/$f is missing"
done
say "game: $GAME"

# ---- tools ----
missing=()
for t in cmake ninja; do command -v "$t" >/dev/null || missing+=("$t"); done
PY="$(command -v python3.11 || command -v python3 || true)"
[ -n "$PY" ] || missing+=("python@3.11")
PREFIX="$(brew --prefix 2>/dev/null || true)"
# sdl2, or sdl2-compat (what newer Homebrew installs for "sdl2"): either leaves
# a CMake package or sdl2-config behind
[ -d "${PREFIX:-/opt/homebrew}/lib/cmake/SDL2" ] || command -v sdl2-config >/dev/null || missing+=("sdl2")
[ ${#missing[@]} -eq 0 ] || die "missing: ${missing[*]}  ->  brew install ${missing[*]}"
xcrun --show-sdk-path >/dev/null 2>&1 || die "no macOS SDK  ->  xcode-select --install"
"$PY" -c 'import sys; sys.exit(sys.version_info < (3, 10))' || die "$PY is too old; need Python 3.10+"

if [ ! -d "$TOOLKIT/pcrecomp/tools" ]; then
    say "fetching submodules"
    git -C "$ROOT" submodule update --init --recursive
fi

if [ ! -x "$VENV/bin/python" ]; then
    say "creating $VENV"
    "$PY" -m venv "$VENV"
fi
"$VENV/bin/python" -m pip install -q --disable-pip-version-check capstone pyelftools
VPY="$VENV/bin/python"

# ---- lift ----
STAGE="$(mktemp -d "${TMPDIR:-/tmp}/gse-lift.XXXXXX")"
trap 'rm -rf "$STAGE"' EXIT
GLH="$(xcrun --show-sdk-path)/System/Library/Frameworks/OpenGL.framework/Headers"

say "lifting the game"
( cd "$TOOLKIT" && "$VPY" -m tools recomp "$GAME/vsg" "$STAGE" )
say "lifting the sound libraries"
# This order fixes where each library lands in guest memory. Keep it.
( cd "$TOOLKIT" && "$VPY" -m tools.recomp.solib "$STAGE" "$GAME/librnalindbergh_jr.so" "$GAME/snd_util.so" )
say "generating GL thunks"
"$VPY" "$TOOLKIT/tools/gen_gl_thunks.py" "$STAGE/recomp_imports.h" "$GLH/gl.h" "$GLH/glext.h" > "$STAGE/recomp_gl_thunks.c"

# copy only what changed, so unchanged files keep their timestamps
mkdir -p "$GEN"
for f in "$STAGE"/*; do
    cmp -s "$f" "$GEN/${f##*/}" || cp "$f" "$GEN/"
done
for f in "$GEN"/*; do
    [ -e "$STAGE/${f##*/}" ] || rm -f "$f"
done

# ---- build ----
say "building"
cmake -S "$ROOT" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      ${PREFIX:+-DCMAKE_PREFIX_PATH="$PREFIX"} >/dev/null
cmake --build "$BUILD"

[ -x "$BUILD/ghostsquadevolution" ] || die "build finished without $BUILD/ghostsquadevolution"
say "built $BUILD/ghostsquadevolution"
printf '\nRun it with:\n  %s/scripts/run-macos.sh "%s"\n' "$ROOT" "$GAME"
