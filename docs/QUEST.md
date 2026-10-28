# Ghost Squad Evolution VR for Meta Quest

The recompiled game running standalone on a Quest 3, in stereo, aimed with a
tracked controller. Branch `quest-port`.

**Status:** runs in the Android XR emulator (arm64, OpenXR, GLES through ANGLE)
from attract mode through menus, cutscenes and Mission 1 gameplay at about
60 fps, with both eyes rendered and the HUD placed correctly. It has **not yet
been run on a physical Quest**, and the controller mapping has not been
exercised (the emulator run had no controllers): input, world scale, comfort
and performance on the XR2 Gen 2 still need a headset.

## Build

Needs the lifted game in `games/ghostsquadevolution/generated` (see the
README), the Android SDK with NDK 27.3.13750724 and build-tools, a JDK 17+,
CMake and Ninja.

```sh
quest/deps.sh      # SDL2, gl4es (patched), OpenAL Soft, OpenXR loader -> ../deps
quest/build.sh     # -> quest/GhostSquadEvolutionVR.apk (about 8 MB, no game data)
```

`DEPS`, `ANDROID_SDK_ROOT`, `ANDROID_NDK` and `JAVA_HOME` override the default
locations.

## Install

Enable developer mode on the headset, connect it with USB and accept
debugging, then give the script the game's `vsg_l` folder (the one the desktop
build runs from):

```sh
quest/install.sh "/path/to/Ghost Squad Evolution/vsg_l"
```

This installs the APK, grants it all-files access, and copies the game (about
570 MB) to `/sdcard/GhostSquadEvolution/vsg_l`. Later updates need only
`quest/install.sh` with no argument. Start **Ghost Squad Evolution VR** from
*Unknown Sources*.

## Controls

| Action | Controller |
| --- | --- |
| Aim and fire | Point and pull the trigger. Either hand: pulling the other trigger moves the gun there |
| Reload | Grip, or pull the trigger while pointing off the screen (the arcade way) |
| Action button (mission level, in-game actions) | A / X on the gun hand |
| Fire mode | B / Y on the gun hand |
| Start | Click either thumbstick |
| Coin | Left menu button |
| Recentre | Hold the Meta button (the game's camera follows) |

The game's sight is drawn on a screen fixed in front of you, the size of the
arcade monitor's view; the controller's ray hits that screen exactly where the
game aims. The world itself fills the headset's whole view.

## Settings

`vr.cfg` in `/sdcard/GhostSquadEvolution/vsg_l`, one setting per line:

```
scale 1.0        game units per metre (world size)
screen 3.0       distance of the HUD screen, metres
resolution 1440  longer side of each eye's image, pixels
refresh 90       display rate to ask for: 72, 80, 90 or 120
hand right       starting gun hand
```

`env.txt` in the same folder sets runtime options, one `NAME=value` per line,
for example `LINDBERGH_MUTE=1`. Logs: `adb shell cat /sdcard/GhostSquadEvolution/gse.log`
or `adb logcat -s gse LIBGL`.

## How it works

The desktop build already runs the game's own code natively; the port adds
three layers between it and the headset.

1. **Stereo** (`lindberghrecomp/src/runtime/stereo.c`). Every GL call the game
   makes is recorded while it draws the first eye; at the swap the recording
   is replayed for the second. Framebuffer 0 becomes the eye's image, render
   targets and viewports are scaled to the eye's resolution, and the game's
   perspective is replaced by the eye's (its pose relative to the game camera
   and its field of view). Screen content (2D, menu 3D, render targets drawn
   as sprites) goes on a virtual screen matching the game's own field of
   view, scissored to its edge; full-screen passes stay full-screen. Cinema
   bars are dropped. `LINDBERGH_STEREO=1` shows both eyes side by side on a
   desktop.
2. **GLES** (Android only). The game is a GL 2.1 fixed-function and
   ARB-assembly program. [gl4es](https://github.com/ptitSeb/gl4es) provides
   GL 2.1 on GLES 3; the runtime's own ARB-to-GLSL translator (`arbprog.c`)
   handles every assembly program, so the driver never sees one. Rectangle
   textures are sampled as 2D with per-unit normalisation.
   `quest/gl4es.patch` fixes two gl4es bugs the game hits.
3. **OpenXR** (`lindberghrecomp/src/runtime/host_xr.c`). Session,
   swapchains, refresh rate, controller actions and haptics. Each game frame
   waits for the headset's next frame, takes the predicted eye poses, and
   submits both eye images.

Audio is OpenAL Soft on OpenSL ES; the APK is SDL2's Java activity plus
`libmain.so`.

## How this follows DR-89's ports

[time-crisis-2-vr](https://github.com/DR-89/time-crisis-2-vr) and
[crisis-zone-vr](https://github.com/DR-89/crisis-zone-vr) take a native
reconstruction of the arcade game and put an OpenXR host around it. This port
has the same shape; the main difference is where the 3D comes from. Their
System 23 renderer is rewritten for VR, while this game is a GL program, so
its own GL stream is re-projected per eye.

| DR-89 port | Here |
| --- | --- |
| `upstream/` native reconstruction (namco22-decompile) | the statically recompiled game (`games/…/generated`) plus `lindberghrecomp` |
| `quest/quest_host.c`: OpenXR instance, GLES binding, swapchains, refresh rate choice, actions, haptics | `host_xr.c`, same structure (Android create-instance and GLES-enable extensions, FB refresh-rate extension, Touch controller profile) |
| `quest/tc2_scene.c` / `tc2_gl.c`: arcade geometry drawn from tracked stereo viewpoints | `stereo.c`: the game's own GL frame recorded and replayed per eye with the eye's projection |
| Ray from the controller to the arcade screen coordinates (`qvr_aim`) | `stereo_ray_to_screen()`, fed to the cabinet's gun position |
| Flat HUD layer on the arcade camera plane | the virtual HUD screen, matched to the game's field of view |
| Cinematic black bars removed in VR | `letterbox_bar()` |
| sRGB swapchain written raw | the same: `GL_SRGB8_ALPHA8` with sRGB writes off |
| Manifest: VR categories, OpenXR permissions and broker queries, `quest3` devices | `quest/app/src/main/AndroidManifest.xml` |
| `tools/build.py`, `Install-Quest.ps1`, `Export-Quest-Diagnostics.ps1` | `quest/build.sh`, `quest/install.sh`, `gse.log` / `env.txt` |
| ROMs bundled or imported | game files copied to shared storage; none ship in the APK |

Not carried over yet: a weapon model in the hand, the in-headset options menu,
physical or grip cover (Ghost Squad has no cover), laser and tracer aim aids,
input recording and replay.

## Debugging on a GL you cannot watch

`LINDBERGH_RT_DUMP=<frame>` writes every render target as the first eye left
it that frame (`rt_*.ppm` in the game folder), and logs the centre pixel after
every draw. Running the same frame on the Mac and on the emulator and
comparing the two logs finds the first draw that differs. That is how the
white-scene bug here was traced to gl4es' `glIsEnabled`.

`LINDBERGH_FRAMEMAP=<frame>` lists one frame's GL calls with what each draw
had bound. `LINDBERGH_SCENE_STATS=1` reports world draws per frame, which is
what tells menus (`LINDBERGH_MENU_DRAWS`, default 120) from the game world.

## Known limitations

- Not yet tested on Quest hardware.
- World scale (`scale`) is a guess of one game unit per metre; it needs
  measuring in the headset, as the TC2 port did.
- Each game frame is one headset frame: the game paces itself at 60 Hz, so
  at 72–120 Hz the compositor's reprojection fills the gaps.
- Menu detection is a draw-count threshold; a sparse moment of gameplay
  could briefly look like a menu.
