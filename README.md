# The House of the Dead 4 — static recompilation for macOS

The House of the Dead 4 (Sega, 2005, Lindbergh) lifted from its x86 Linux
executable to C with [lindberghrecomp](https://github.com/lialan/lindberghrecomp),
and built as a native macOS program. It plays: attract mode, credits, the
missions, with sound, in English, at the cabinet's 1280x768.

No game files are in this repository, and neither is the lifted code: you need
your own copy of the game (the folder holding `elf/hod4M.elf` and `fs/`).

## Build and run (macOS)

```sh
brew install cmake ninja sdl2 python@3.11
git clone --recursive https://github.com/lialan/hotd4-lindbergh-recomp.git
cd hotd4-lindbergh-recomp
./scripts/build-macos.sh "/path/to/House of the Dead 4"
./scripts/run-macos.sh   "/path/to/House of the Dead 4"
```

## Controls

| | |
| --- | --- |
| Aim | mouse |
| Fire | left button |
| Reload | right button, or move the pointer off the window (the game wants the gun shaken; this shakes it) |
| Coin / Start | 5 / 1 (two credits start a game) |
| Mute | M, or the Sound menu |

The Language menu picks English (the default) or Japanese, from the next start.
`LINDBERGH_MUTE=1` keeps the game silent for a whole run.
