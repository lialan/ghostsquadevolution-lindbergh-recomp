Ghost Squad Evolution VR for Meta Quest 3, Quest 3S and Quest Pro.

**The game itself is not included.** This APK holds the port: the game's code
recompiled to run natively on the headset, the VR host, and its libraries.
You need your own copy of the arcade game's `vsg_l` folder.

## Install

1. Enable developer mode on the headset and install the APK with
   [SideQuest](https://sidequestvr.com) or `adb install -r <apk>`.
2. Copy your whole `vsg_l` folder to the headset's shared storage as
   `GhostSquadEvolution/vsg_l`, so the game executable is at
   `/sdcard/GhostSquadEvolution/vsg_l/vsg`. Over USB (allow file access in
   the headset) that is *Internal shared storage › GhostSquadEvolution*;
   SideQuest's file browser works too, or:
   `adb push vsg_l /sdcard/GhostSquadEvolution/`
3. Start **Ghost Squad Evolution VR** from *Unknown Sources*. The first time
   it opens the system screen to allow access to all files: allow it, then
   start the app again. (Or grant it from a computer:
   `adb shell appops set --uid org.lindbergh.gse.vr MANAGE_EXTERNAL_STORAGE allow`.)

The port only runs the exact game version it was built from: `vsg` with
SHA-256 `2966eed7254b485bd086215df226240ae97464132714a1a1504c52ba2a9e4815`.
If the folder is missing or a different version, the app closes and explains
why in `Android/data/org.lindbergh.gse.vr/files/HOW-TO-ADD-THE-GAME.txt`.

Updates install over the previous version and keep the game files, settings
and saved cabinet data.

## Play

Trigger fires, grip reloads (or pull the trigger pointing off the screen), a
thumbstick click is Start, and coins are automatic. The left controller's
menu button opens the settings panel: sights, laser, gun model, gun hand,
resolution, display rate and more.

More: [docs/QUEST.md](https://github.com/lialan/ghostsquadevolution-lindbergh-recomp/blob/quest-port/docs/QUEST.md).
