/*
 * host.c - run recompiled Ghost Squad Evolution.
 *
 * There is very little to do here, and that is the point. The runtime maps the
 * game where it was linked and builds the stack the kernel would have; the
 * lifted code is the game. This just hands control over at the ELF's entry
 * point and gets out of the way.
 *
 * Everything the game reaches for that is not its own code - libc, OpenGL,
 * sound, the JVS I/O board - arrives as hle_call(), and the handlers live in
 * hle_*.c next to this file. A handler that is missing aborts naming itself,
 * so the way to work on this port is to run it and read what it asks for.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lindbergh_rt.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr,
            "usage: %s <vsg> [game args...]\n"
            "\n"
            "  vsg is Ghost Squad Evolution's executable, from a game tree you\n"
            "  can already read. None of it ships here - see the README.\n",
            argv[0]);
        return 2;
    }

    /* Before anything guest-shaped runs, so a fault in lifted code reports
     * the simulated machine rather than a C function name in a generated file. */
    guest_install_crash_handler();

    if (guest_load(argv[1], argc - 1, argv + 1) != 0)
        return 1;

#ifndef __ANDROID__
    /* 16:9 by default on the desktop (LINDBERGH_WIDE=0 is the arcade's 4:3):
     * the world seen wider, the HUD at the edges, menus as the arcade's 4:3.
     * The game culls its models to its own 4:3 view, so that is widened to
     * match - about 62 degrees vertical holds a 16:9 view's width. */
    {
        const char *w = getenv("LINDBERGH_WIDE");
        if (!w || !*w) { setenv("LINDBERGH_WIDE", "1", 1); w = "1"; }
        if (*w != '0') setenv("LINDBERGH_CULL_FOV", "62", 0);
    }
#endif
    /* Give the imports bodies. Everything the toolkit ships is bound here;
     * anything this game needs beyond that gets bound on top, and whatever is
     * still missing aborts naming itself on first call. */
    hle_register_all();

    CPU cpu;
    guest_init_cpu(&cpu);

    /* The guest never returns here: its exit path is the exit_group syscall,
     * which the runtime turns into the host's exit(). Reaching the line below
     * means the entry point returned, which a Linux _start does not do. */
    dispatch(&cpu, guest_entry());

    fprintf(stderr, "[host] the entry point returned - _start should not.\n");
    return 1;
}
