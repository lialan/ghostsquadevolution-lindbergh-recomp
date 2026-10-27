cmake_minimum_required(VERSION 3.20)
project(ghostsquadevolution VERSION 0.1.0 LANGUAGES C)

# Ghost Squad Evolution (Sega, 2007), statically recompiled.
#
# The game target only exists once you have lifted an ELF, because the runtime
# needs two headers generated from it. Without them this still configures and
# builds the toolkit's own self-check, so the repo is testable out of the box.
#
#   py -3.11 -m tools recomp vsg ..\games\ghostsquadevolution\generated
#   cmake -S . -B build -A Win32
#   cmake --build build --config Release

add_subdirectory(lindberghrecomp)

set(GEN "${CMAKE_CURRENT_SOURCE_DIR}/games/ghostsquadevolution/generated")

file(GLOB LIFTED "${GEN}/recomp_funcs_*.c")
if(LIFTED AND EXISTS "${GEN}/recomp_imports.h")
    add_executable(ghostsquadevolution
        games/ghostsquadevolution/src/host.c
        ${LIFTED}
    )
    if(NOT WIN32)
        # Typed GL thunks for a 64-bit host, generated from the host's GL headers
        # (lindberghrecomp/tools/gen_gl_thunks.py).
        target_sources(ghostsquadevolution PRIVATE "${GEN}/recomp_gl_thunks.c")
        # the dlopened sound libraries, lifted by lindberghrecomp/tools/recomp/solib.py
        file(GLOB SOLIB "${GEN}/recomp_solib_*.c")
        target_sources(ghostsquadevolution PRIVATE ${SOLIB})
    endif()
    lindbergh_runtime(ghostsquadevolution "${GEN}")
    message(STATUS "ghostsquadevolution: building against lifted code in ${GEN}")
else()
    message(STATUS
        "ghostsquadevolution: no lifted code in ${GEN} yet, building the "
        "toolkit self-check only. See the README.")
endif()
