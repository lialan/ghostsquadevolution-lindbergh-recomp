// Ghost Squad Evolution VR for Meta Quest. All of the game runs natively
// (libmain.so, see games/ghostsquadevolution/src/android_main.c); this only
// prepares the folder the game runs from and loads the libraries.
//
// /sdcard/GhostSquadEvolution (needs all-files access, which
// quest/install.sh grants):
//   vsg_l/      the game, copied there with adb (quest/install.sh)
//   gse.log     what the runtime did on the last run
package org.lindbergh.gse;

import android.os.Bundle;
import org.libsdl.app.SDLActivity;

public class GhostSquadVR extends SDLActivity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL2", "openal", "openxr_loader", "main" };
    }
}
