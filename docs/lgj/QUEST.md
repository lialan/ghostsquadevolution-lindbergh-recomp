# Let's Go Jungle! VR for Meta Quest

The recompiled *Let's Go Jungle! Lost on the Island of Spice* (the 2006
original, not *Special*) running standalone on a Quest 3, in stereo, aimed
with a tracked controller. It is the Ghost Squad Evolution VR port's runtime
(`lindberghrecomp`) with what this game needed on top.

**Status:** plays on a Quest 3 from attract mode through the first stage at
60 fps: the jungle in 3D around you, the HUD on a screen in front of you.
Checked from captured eye images of muted, scripted runs; not yet played with
real controllers. Not in a release yet: build it yourself.

## Build

Needs the lifted game in `games/lgj/generated` (see the README), the Android
SDK with NDK 27.3.13750724 and build-tools, a JDK 17+, CMake and Ninja.

```sh
quest/deps.sh      # SDL2, gl4es (patched), OpenAL Soft, OpenXR loader -> ../deps
# the GL thunks for the Quest: every entry point gl4es defines
G=games/lgj/generated
$NDK/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-nm -g --defined-only ../deps/gl4es/lib/libGL.a \
    | awk 'NF==3{print $3}' | sort -u > /tmp/gl4es.exports
python3 lindberghrecomp/tools/gen_gl_thunks.py --exports /tmp/gl4es.exports \
    $G/recomp_imports.h ../deps/gl4es/include/GL/gl.h ../deps/gl4es/include/GL/glext.h \
    > $G/recomp_gl_thunks_android.c
shasum -a 256 "/path/to/Let's Go Jungle/lgj_final" | cut -d' ' -f1 > $G/lgj_final.sha256
quest/build.sh lgj     # -> quest/LetsGoJungleVR.apk (about 14 MB, no game data)
```

## Install

Enable developer mode on the headset, connect it with USB and accept
debugging, then give the script the game's folder (the one holding
`lgj_final`, `data`, `shader` and `extraShader`):

```sh
quest/install.sh lgj "/path/to/Let's Go Jungle"
```

This installs the APK, grants it all-files access and copies the game (about
1 GB) to `/sdcard/LetsGoJungle`. Later updates need only
`quest/install.sh lgj`. Start **Let's Go Jungle VR** from *Unknown Sources*.
The app refuses any `lgj_final` other than the one the code was lifted from
(by SHA-256).

## Controls

As in The House of the Dead 4 VR: point and pull the trigger to fire (either
hand; the jungle guns never need reloading), a thumbstick click is Start,
coins are automatic, and the left controller's menu button opens the settings
panel (with *Reset game*).

## Settings

`env.txt` in `/sdcard/LetsGoJungle` sets runtime options, one `NAME=value`
per line, for example `LINDBERGH_MUTE=1`. Log of the last run:
`adb shell cat /sdcard/LetsGoJungle/lgj.log`.

| Option | |
| --- | --- |
| `LINDBERGH_FLAT_ALL=1` | the whole picture on the screen, as on the cabinet |
| `LINDBERGH_EYE_DUMP=<first>,<count>[,<stride>]` | write the eyes' images (`eye_<frame>.ppm`, `eyeR_...`) |

## What this game needed beyond the other two

- **Its camera is in its shader constants.** The game never uses GL's
  matrices: each vertex program gets its model-view-projection as
  `program.env[0..3]`. The stereo layer, which tells the world from the HUD
  and builds each eye's view from the game's projection, now takes that
  projection apart from the matrix before each draw (`arb_env_projection`).
- **Its picture is an 800x600 target.** The scene is drawn in half floats
  into an 800x600 texture and tone mapped onto the 1360x768 screen. That
  target is drawn at the eyes' size in both views, and the tone-mapping pass
  reads it per view (gl4es binds its two-layer array).
- **Shadow maps.** The Quest's driver silently dropped every draw that read a
  shadow sampler - the ground, trees and leaves. The comparison is done in
  the shader on a plain sampler instead.
- **Its programs.** Cg's skinning programs copied 224 constants into a local
  array for every vertex (4 fps); they read the constants directly now.
  Programs without branches no longer run in a 4096-step loop. Back colours
  are written only by programs that set them: gl4es used them for every
  back face, and the HUD came out black.
- **gl4es.** Half-float render targets stay half float, depth is copied back
  from multiview layers, and `shadow2D`/`shadow2DProj` exist in its
  multiview shaders (`quest/gl4es.patch`).
- **Its sound library** unlocks mutexes from threads that did not lock them,
  which the arcade's Linux allows for a plain mutex. The runtime made every
  mutex recursive (from the Windows side), and start-up sometimes failed in
  `CRICRS_Leave`. Mutexes now have the type the game asks for.
