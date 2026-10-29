#!/bin/sh
# Install the APK on a headset over adb and copy the game to it
# (/sdcard/HouseOfTheDead4: elf, fs and drv).
#
#   quest/install.sh "/path/to/House of the Dead 4"
#
# The game folder is the one the desktop build runs from (with elf/hod4M.elf
# in it); about 2.8 GB is copied once. Run again with no argument to update
# only the APK.
set -e
Q=$(cd "$(dirname "$0")" && pwd)
PKG=org.lindbergh.hotd4.vr
DEST=/sdcard/HouseOfTheDead4
adb install -r "$Q/HouseOfTheDead4VR.apk"
# the game is read from shared storage
adb shell appops set --uid $PKG MANAGE_EXTERNAL_STORAGE allow
if [ -n "$1" ]; then
    [ -f "$1/elf/hod4M.elf" ] || { echo "$1 has no elf/hod4M.elf - give the game's folder"; exit 1; }
    adb shell mkdir -p "$DEST"
    for d in elf fs drv; do adb push "$1/$d" "$DEST/"; done
fi
echo "Installed. Start \"The House of the Dead 4 VR\" from Unknown Sources."
echo "Log of the last run: adb shell cat $DEST/hotd4.log"
