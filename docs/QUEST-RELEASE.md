Sega Lindbergh arcade games in VR, for Meta Quest 3, Quest 3S and Quest Pro:

| Game | APK |
| --- | --- |
| Ghost Squad Evolution | `GhostSquadEvolutionVR-<version>-quest3.apk` |
| The House of the Dead 4 | `HouseOfTheDead4VR-<version>-quest3.apk` |

**The games themselves are not included.** Each APK holds a port: the game's
code recompiled to run natively on the headset, the VR host, and its
libraries. You need your own copy of the arcade game.

## Install

1. Enable developer mode on the headset and install the APK with
   [SideQuest](https://sidequestvr.com) or `adb install -r <apk>`.
2. Copy the game to the headset's shared storage (over USB, allow file
   access in the headset: *Internal shared storage*; SideQuest's file browser
   works too):
   - **Ghost Squad Evolution:** your whole `vsg_l` folder as
     `GhostSquadEvolution/vsg_l`, so the executable is at
     `/sdcard/GhostSquadEvolution/vsg_l/vsg`
     (`adb push vsg_l /sdcard/GhostSquadEvolution/`).
   - **The House of the Dead 4:** the game's `elf`, `fs` and `drv` folders
     into `HouseOfTheDead4`, so the executable is at
     `/sdcard/HouseOfTheDead4/elf/hod4M.elf`
     (`adb push elf fs drv /sdcard/HouseOfTheDead4/`).
3. Start the game from *Unknown Sources*. The first time it opens the system
   screen to allow access to all files: allow it, then start the app again.
   (Or grant it from a computer:
   `adb shell appops set --uid <package> MANAGE_EXTERNAL_STORAGE allow`, with
   `org.lindbergh.gse.vr` or `org.lindbergh.hotd4.vr`.)

Each port only runs the exact game version it was built from:

| Game | Executable | SHA-256 |
| --- | --- | --- |
| Ghost Squad Evolution | `vsg` | `2966eed7254b485bd086215df226240ae97464132714a1a1504c52ba2a9e4815` |
| The House of the Dead 4 | `elf/hod4M.elf` | `72fdd45830acd9317ec1e4f97de35a270c94746068fdaf827381c91bb47aa296` |

If the game is missing or a different version, the app closes and explains
why in `Android/data/<package>/files/HOW-TO-ADD-THE-GAME.txt`.

Updates install over the previous version and keep the game files, settings
and saved cabinet data.

## Play

Trigger fires, grip reloads (or pull the trigger pointing off the screen), a
thumbstick click is Start, and coins are automatic. The left controller's
menu button opens the settings panel: sights, laser, gun model, gun hand,
resolution, display rate, recentring, and Reset game.

More: [Ghost Squad Evolution](https://github.com/lialan/lindbergh-recomp-ports/blob/main/docs/ghostsquadevolution/QUEST.md),
[The House of the Dead 4](https://github.com/lialan/lindbergh-recomp-ports/blob/main/docs/hotd4/QUEST.md).
