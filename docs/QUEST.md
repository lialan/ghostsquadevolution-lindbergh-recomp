# The House of the Dead 4 VR for Meta Quest

The recompiled game running standalone on a Quest 3, in stereo, aimed with a
tracked controller. Branch `quest-port`. It is the Ghost Squad Evolution VR
port's runtime (`lindberghrecomp`, branch `hotd4-quest`) with what this game
needed on top.

**Status:** runs on a Quest 3 from attract mode into stage 1 gameplay, both
eyes drawn in one pass, at 50-60 fps: the world in 3D around you, the HUD on a
screen in front of you. Checked from captured eye images with a scripted
start; aiming, reloading and sound have not been played with real controllers
yet.

## Build

Needs the lifted game in `games/hotd4/generated` (see the README), the
Android SDK with NDK 27.3.13750724 and build-tools, a JDK 17+, CMake and Ninja.

```sh
quest/deps.sh      # SDL2, gl4es (patched), OpenAL Soft, OpenXR loader -> ../deps
# the GL thunks for the Quest: every entry point gl4es defines
G=games/hotd4/generated
$NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-nm -g --defined-only ../deps/gl4es/lib/libGL.a \
    | awk 'NF==3{print $3}' | sort -u > /tmp/gl4es.exports
python3 lindberghrecomp/tools/gen_gl_thunks.py --exports /tmp/gl4es.exports \
    $G/recomp_imports.h ../deps/gl4es/include/GL/gl.h ../deps/gl4es/include/GL/glext.h \
    > $G/recomp_gl_thunks_android.c
shasum -a 256 "/path/to/House of the Dead 4/elf/hod4M.elf" | cut -d' ' -f1 > $G/hod4M.elf.sha256
quest/build.sh     # -> quest/HouseOfTheDead4VR.apk (about 12 MB, no game data)
```

`DEPS`, `ANDROID_SDK_ROOT`, `ANDROID_NDK` and `JAVA_HOME` override the default
locations. `quest/build.sh --with-game <game folder>` puts the game's files
inside the APK instead (about 3 GB; for your own headset only).

## Install

Enable developer mode on the headset, connect it with USB and accept
debugging, then give the script the game's folder (the one holding
`elf/hod4M.elf`, `fs` and `drv`):

```sh
quest/install.sh "/path/to/House of the Dead 4"
```

This installs the APK, grants it all-files access and copies the game (about
2.8 GB) to `/sdcard/HouseOfTheDead4`. Later updates need only
`quest/install.sh` with no argument. Start **House of the Dead 4 VR** from
*Unknown Sources*. The app refuses any `hod4M.elf` other than the one the code
was lifted from (by SHA-256).

## Controls

| Action | Controller |
| --- | --- |
| Aim and fire | Point and pull the trigger. Either hand: pulling the other trigger moves the gun there |
| Reload | Grip, or pull the trigger while pointing off the screen (the cabinet's gun is shaken; this shakes it) |
| Start | Click either thumbstick; with automatic coins this also inserts the coins |
| Laser / dot / off | Hold the upper button (B / Y) on the other hand for a second |
| Recentre | Hold the Meta button |

## Settings

`vr.cfg` in `/sdcard/HouseOfTheDead4/elf` holds the headset settings
(world scale, HUD screen distance, resolution, refresh rate, gun hand, laser,
coins), as in Ghost Squad Evolution VR. `env.txt` in `/sdcard/HouseOfTheDead4`
sets runtime options, one `NAME=value` per line, for example
`LINDBERGH_MUTE=1`. Log of the last run:
`adb shell cat /sdcard/HouseOfTheDead4/hotd4.log`.

| Option | |
| --- | --- |
| `LINDBERGH_CULL_WIDEN=4` | how much wider than the game's camera its clip test is (1: the game's own) |
| `LINDBERGH_FLAT_ALL=1` | the whole picture on the screen, as on the cabinet |
| `LINDBERGH_EYE_DUMP_EVERY=N` | write the first eye every N frames (`eye_<frame>.ppm` in `elf`) |

## What this game needed beyond Ghost Squad

- **Its shaders.** The NN library's GLSL tests option macros with `#if NAME`
  where NAME is often not defined - 0 in desktop GLSL, an error in GLSL ES.
  The runtime decides every `#if` the way a desktop compiler does before
  gl4es sees the source (`glsl_pp.c`). gl4es also sized `gl_TextureMatrix[]`
  as -1 when indexed by a macro, and uploaded every element of that array as
  unit 0's matrix, so decal, normal and specular maps sampled one texel
  (`quest/gl4es.patch`).
- **Its pbuffer.** The game draws its 3D into a GLX pbuffer, copies it into a
  texture and draws that over the window under its HUD. In the headset the
  pbuffer is the eyes, like the window: the scene is drawn into them directly,
  and the copy laid back over it is left out. Copies of the window
  (`glCopyTexSubImage2D`, the glare's small copy) take the eye image's region.
- **Side passes into the pbuffer.** Besides the scene, the game draws other
  views into its pbuffer to copy them into textures - the glare at 80x60,
  and in some rooms three 512x512 reflection passes after the scene, each
  cleared first. With the pbuffer being the eyes those clears wiped the scene,
  leaving the clear colour under the HUD. A pass cleared with less than the
  whole screen as its viewport now goes into a framebuffer of its own.
- **Fetched entry points.** It asks GLX for `glMultiDrawElements`,
  `glActiveTexture` and others by name; calls through those pointers now pass
  the stereo layer like imported ones.
- **Culling.** The NN clip test (`nnSetClipPlane`) is built for a view four
  times wider than the game's camera, so scenery and enemies stay drawn when
  you look around.
