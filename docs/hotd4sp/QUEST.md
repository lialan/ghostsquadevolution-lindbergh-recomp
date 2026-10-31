# The House of the Dead 4 Special VR for Meta Quest

The recompiled *The House of the Dead 4 Special* (2006, the ride cabinet's
version) running standalone on a Quest 3, in stereo, aimed with a tracked
controller. It is the same runtime as the other ports (`lindberghrecomp`),
with what this game needed on top.

**Status:** plays on a Quest 3 from attract mode through the stages, path
choices included, at 55-60 fps. Played with real controllers.

## Install

Enable developer mode on the headset and install
`HouseOfTheDead4SpecialVR-<version>-quest3.apk` from the
[releases page](https://github.com/lialan/lindbergh-recomp-ports/releases)
with [SideQuest](https://sidequestvr.com) or `adb install -r <apk>`. Then copy
the game's `elf` and `fs` folders into `/sdcard/HouseOfTheDead4Special`, so
the executable is at `/sdcard/HouseOfTheDead4Special/elf/hod4M.elf`:

```sh
adb push elf fs /sdcard/HouseOfTheDead4Special/
```

Start **The House of the Dead 4 Special VR** from *Unknown Sources*; the first
time, allow all-files access and start it again. The app refuses any
`hod4M.elf` other than the one the code was lifted from (by SHA-256).

## Controls

Point and pull the trigger to fire; shake the gun to reload (or press the
grip); a thumbstick click is Start; coins are automatic. At a branch, shoot
one of the two boxes to choose the path. The left controller's menu button
opens the settings panel, which adds a **HUD position** row for this game:
the HUD stays in front of your face, and each pull moves it (centre, down
10/20/30 degrees, up 10).

## Build

Needs the lifted game in `games/hotd4sp/generated` (the macOS build makes
it: `scripts/build-macos.sh hotd4sp "/path/to/the game"`), the Android SDK
with NDK 27.3.13750724 and build-tools, a JDK 17+, CMake and Ninja; the GL
thunks for the Quest are made as in [Let's Go Jungle!'s doc](../lgj/QUEST.md).

```sh
quest/deps.sh
quest/build.sh hotd4sp            # -> quest/HouseOfTheDead4SpecialVR.apk, no game data
quest/install.sh hotd4sp "/path/to/the game"
```

## Settings

`env.txt` in `/sdcard/HouseOfTheDead4Special` sets runtime options, one
`NAME=value` per line, for example `LINDBERGH_MUTE=1`. Log of the last run:
`adb shell cat /sdcard/HouseOfTheDead4Special/hotd4sp.log`.

## What this game needed beyond the others

- **A stripped executable.** It has no symbol table. The lifter finds its
  functions from the code and its unwind tables (`tools bounds`), and names
  the ones the runtime needs by matching them against the original
  *The House of the Dead 4*.
- **The cabinet.** Its keychip check, the ride seat's motion board (emulated
  on the first serial port, motion off), the I/O board's shake switch, and
  the region in its EEPROM (English).
- **Speed.** Its street scenes ran at 32 fps on the headset. The GL calls now
  run on a thread of their own, the lifter keeps registers in locals and
  drops flag work nothing reads, and calls between known functions go
  straight to them.
- **Its 2D camera.** The HUD, subtitles, movies and the path choice's boxes
  are quads before a camera of their own; they are kept on a screen that
  follows your head, movies keep the depth that hides what is behind them,
  and greyscale movie planes are uploaded so gl4es keeps them.
- **The gun.** Each shot leaves the gun itself: the game's shot ray is
  replaced by the controller's, so hits follow the laser at any distance,
  and the hit splash is drawn where the laser crosses the HUD.
