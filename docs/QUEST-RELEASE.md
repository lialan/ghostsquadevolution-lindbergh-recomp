Ghost Squad Evolution VR for Meta Quest 3, Quest 3S and Quest Pro.

**The game itself is not included.** This APK holds the port: the game's code
recompiled to run natively on the headset, the VR host, and its libraries.
You need your own copy of the arcade game's `vsg_l` folder.

## Install

1. Enable developer mode on the headset and install the APK with
   [SideQuest](https://sidequestvr.com) or `adb install -r <apk>`.
2. Start **Ghost Squad Evolution VR** once from *Unknown Sources*. It closes
   straight away and creates its folder on the headset.
3. Connect the headset to a computer by USB, allow file access in the
   headset, and copy your whole `vsg_l` folder into
   *Internal shared storage › Android › data › org.lindbergh.gse.vr › files*,
   so the game executable ends up at `…/files/vsg_l/vsg`. SideQuest's file
   browser works as well, or:
   `adb push vsg_l /sdcard/Android/data/org.lindbergh.gse.vr/files/`
4. Start the app again.

The port only runs the exact game version it was built from: `vsg` with
SHA-256 `2966eed7254b485bd086215df226240ae97464132714a1a1504c52ba2a9e4815`.
If the folder is missing or a different version, the app closes and explains
why in `HOW-TO-ADD-THE-GAME.txt` in that same folder.

Updates install over the previous version and keep the game files, settings
and saved cabinet data.

## Play

Trigger fires, grip reloads (or pull the trigger pointing off the screen), a
thumbstick click is Start, and coins are automatic. The left controller's
menu button opens the settings panel: sights, laser, gun model, gun hand,
resolution, display rate and more.

More: [docs/QUEST.md](https://github.com/lialan/ghostsquadevolution-lindbergh-recomp/blob/quest-port/docs/QUEST.md).
