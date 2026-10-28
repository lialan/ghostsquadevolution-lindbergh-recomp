#!/bin/sh
# Build the Meta Quest APK: quest/GhostSquadEvolutionVR.apk
#
#   quest/build.sh
#
# Needs the lifted game in games/ghostsquadevolution/generated (see the
# README), the Android SDK with NDK 27, a JDK 17+, and the dependencies built
# for arm64 Android in $DEPS - see docs/QUEST.md. The APK carries no game
# files; quest/install.sh copies them to the headset.
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
Q=$ROOT/quest
SDK=${ANDROID_SDK_ROOT:-$HOME/Library/Android/sdk}
NDK=${ANDROID_NDK:-$SDK/ndk/27.3.13750724}
DEPS=${DEPS:-$ROOT/../deps}
SDL_SRC=${SDL_SRC:-$DEPS/SDL2-2.32.10}
export JAVA_HOME=${JAVA_HOME:-/opt/homebrew/opt/openjdk@21/libexec/openjdk.jdk/Contents/Home}
case "$(uname -s)" in Darwin) HOST=darwin-x86_64 ;; *) HOST=linux-x86_64 ;; esac
STRIP=$NDK/toolchains/llvm/prebuilt/$HOST/bin/llvm-strip

cmake -S "$ROOT" -B "$ROOT/build-quest" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$NDK/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-29 -DCMAKE_BUILD_TYPE=Release \
    -DSDL2_DIR="$DEPS/sdl2-android/lib/cmake/SDL2" \
    -DGL4ES_DIR="$DEPS/gl4es" -DGL4ES_LIB="$DEPS/gl4es/lib/libGL.a" \
    -DOPENAL_DIR="$DEPS/openal-soft" -DOPENAL_LIB="$DEPS/openal-android/libopenal.so" \
    -DOPENXR_DIR="$DEPS/oxr/prefab/modules/headers" -DOPENXR_LIB="$DEPS/oxr/jni/arm64-v8a/libopenxr_loader.so" >/dev/null
cmake --build "$ROOT/build-quest" -j"$(getconf _NPROCESSORS_ONLN)"

LIBS=$Q/app/src/main/libs/arm64-v8a
mkdir -p "$LIBS"
"$STRIP" --strip-unneeded -o "$LIBS/libmain.so" "$ROOT/build-quest/libmain.so"
"$STRIP" --strip-unneeded -o "$LIBS/libopenal.so" "$DEPS/openal-android/libopenal.so"
cp "$DEPS/sdl2-android/lib/libSDL2.so" "$DEPS/oxr/jni/arm64-v8a/libopenxr_loader.so" "$LIBS/"
rm -rf "$Q/app/src/main/java/org/libsdl"
cp -R "$SDL_SRC/android-project/app/src/main/java/org/libsdl" "$Q/app/src/main/java/org/"
echo "sdk.dir=$SDK" > "$Q/local.properties"

cd "$Q"
./gradlew --no-daemon -q assembleRelease
cp app/build/outputs/apk/release/app-release.apk "$Q/GhostSquadEvolutionVR.apk"
ls -la "$Q/GhostSquadEvolutionVR.apk"
