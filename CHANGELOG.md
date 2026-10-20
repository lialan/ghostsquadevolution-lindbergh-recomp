# Changelog

All notable changes to this project. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow
[SemVer](https://semver.org/). Nothing has been tagged yet. Everything so far
is under Unreleased, taken from the commits.

## [Unreleased]

### Added
- The host (`games/ghostsquadevolution/src/host.c`) and the build: lift the
  ELF, link it against lindberghrecomp, run it from the game's directory.
- The game boots past its own heap manager and GLUT setup into its boot
  sequence. It renders at 640x480, takes a coin and start, and reaches the SEGA
  warning screen. Input (coin, start, trigger, gun) reaches the game.
- `CHANGELOG.md`, `ROADMAP.md`, `CONTRIBUTING.md`, CI that builds and runs the
  toolkit's self-check on every push and PR, and a netlab recipe.

### Changed
- The toolkit is pinned to lindberghrecomp `d65e3b4` (lindberghrecomp#1, #2).
  The game still lifts (6,033 functions, 298 imports) and builds against it.
- The README follows the house layout: a Getting Started from a clean machine,
  Usage, Building from source, License and Contributors.

### Known
- The process dies with a fail-fast (`0xC0000409`) a few hundred frames after
  the warning screen. See the README for what has been ruled out.
