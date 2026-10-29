cmake_minimum_required(VERSION 3.20)
project(hotd4 VERSION 0.1.0 LANGUAGES C)
if(APPLE)
    enable_language(OBJC)       # the runtime's macOS menu bar
endif()

# The House of the Dead 4 (Sega, 2005), statically recompiled.
#
# The game target only exists once you have lifted an ELF, because the runtime
# needs two headers generated from it. Without them this still configures and
# builds the toolkit's own self-check, so the repo is testable out of the box.
#
#   py -3.11 -m tools recomp hod4M.elf ..\games\hotd4\generated
#   cmake -S . -B build -A Win32
#   cmake --build build --config Release

add_subdirectory(lindberghrecomp)

set(GEN "${CMAKE_CURRENT_SOURCE_DIR}/games/hotd4/generated")

file(GLOB LIFTED "${GEN}/recomp_funcs_*.c")
if(LIFTED AND EXISTS "${GEN}/recomp_imports.h")
    add_executable(hotd4
        games/hotd4/src/host.c
        ${LIFTED}
    )
    if(NOT WIN32)
        # Typed GL thunks for a 64-bit host, generated from the host's GL headers
        # (lindberghrecomp/tools/gen_gl_thunks.py).
        target_sources(hotd4 PRIVATE "${GEN}/recomp_gl_thunks.c")
        # the dlopened sound libraries, lifted by lindberghrecomp/tools/recomp/solib.py
        file(GLOB SOLIB "${GEN}/recomp_solib_*.c")
        target_sources(hotd4 PRIVATE ${SOLIB})
    endif()
    lindbergh_runtime(hotd4 "${GEN}")
    message(STATUS "hotd4: building against lifted code in ${GEN}")
else()
    message(STATUS
        "hotd4: no lifted code in ${GEN} yet, building the "
        "toolkit self-check only. See the README.")
endif()
