#!/bin/sh
# Install the APK on a headset over adb and copy the game to it
# (/sdcard/GhostSquadEvolution/vsg_l).
#
#   quest/install.sh "/path/to/Ghost Squad Evolution/vsg_l"
#
# The game folder is the one the desktop build runs from (vsg_l, with the
# vsg executable in it); about 560 MB is copied once. Run again with no
# argument to update only the APK.
set -e
Q=$(cd "$(dirname "$0")" && pwd)
PKG=org.lindbergh.gse.vr
DEST=/sdcard/GhostSquadEvolution
adb install -r "$Q/GhostSquadEvolutionVR.apk"
# the game is read from shared storage
adb shell appops set --uid $PKG MANAGE_EXTERNAL_STORAGE allow
if [ -n "$1" ]; then
    [ -f "$1/vsg" ] || { echo "$1 has no vsg executable - give the vsg_l folder"; exit 1; }
    adb shell mkdir -p "$DEST/vsg_l"
    # the contents, so the folder on the headset is vsg_l whatever it is called here
    (cd "$1" && for f in * .[!.]*; do [ -e "$f" ] && adb push "$f" "$DEST/vsg_l/"; done)
fi
echo "Installed. Start \"Ghost Squad Evolution VR\" from Unknown Sources."
echo "Log of the last run: adb shell cat $DEST/gse.log"
