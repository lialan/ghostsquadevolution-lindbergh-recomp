/*
 * host.c - run recompiled Let's Go Jungle! Lost on the Island of Spice.
 *
 * There is very little to do here, and that is the point. The runtime maps the
 * game where it was linked and builds the stack the kernel would have; the
 * lifted code is the game. This just hands control over at the ELF's entry
 * point and gets out of the way.
 *
 * Everything the game reaches for that is not its own code - libc, OpenGL,
 * Cg, sound, the JVS I/O board - arrives as hle_call(), and the handlers live
 * in hle_*.c next to this file. A handler that is missing aborts naming
 * itself, so the way to work on this port is to run it and read what it asks
 * for.
 */

#include <stdio.h>
#include <stdlib.h>

#include "game_profile.h"
#include "lindbergh_rt.h"


/* The gun's calibration. Per analog channel (P1 x, y, P2 x, y) the cabinet
 * keeps the readings at the screen's two edges, set in the test menu and
 * stored with the backup, and clamps the gun to them; a fresh backup has
 * zeros there, which pinned every shot to the top-right corner. The runtime
 * reports the gun across the screen as a 16-bit reading (hle_sega.c), so an
 * uncalibrated board gets that whole range. Returns the array. */
extern void (*dispatch_lifted(uint32_t va))(CPU *);
static void (*g_analog_adjust)(CPU *);
static void h_analog_adjust(CPU *c)
{
    push32(c, 0xDEADBEEFu);             /* the return slot its ret pops */
    g_analog_adjust(c);
    uint32_t a = c->eax;
    int blank = 1;
    for (int i = 0; i < 8; i++) if (rd16(a + 2u * (uint32_t)i)) blank = 0;
    if (blank)
        for (int i = 0; i < 8; i++) wr16(a + 2u * (uint32_t)i, (uint16_t)(i & 1 ? 0xFFFF : 0));
}

/* Where the sight lands for a gun reading. The game reads the gun as two
 * angles of a gun on a pivot, so the sight's x follows the reading the
 * runtime sends as y, its y follows x the other way round, and both bend
 * towards the edges (a tangent). Measured with the calibration above, the
 * reading in game pixels (1360x768): */
static const short k_sx[][2] = {           /* sight x <- y reading */
    { 26, 0 }, { 86, 50 }, { 328, 200 }, { 483, 300 }, { 606, 384 }, { 708, 450 },
    { 878, 550 }, { 1075, 650 }, { 1304, 750 }, { 1328, 767 },
};
static const short k_sy[][2] = {           /* sight y <- x reading */
    { 24, 1359 }, { 88, 1250 }, { 152, 1150 }, { 326, 850 }, { 406, 700 },
    { 484, 550 }, { 637, 250 }, { 663, 150 }, { 768, 0 },
};
static int interp(const short (*t)[2], int n, int v)
{
    if (v <= t[0][0]) return t[0][1];
    for (int i = 1; i < n; i++)
        if (v <= t[i][0])
            return t[i - 1][1] + (t[i][1] - t[i - 1][1]) * (v - t[i - 1][0]) / (t[i][0] - t[i - 1][0]);
    return t[n - 1][1];
}
/* The pointer (where the sight should be) -> the readings that put it there. */
static void gun_map(int *x, int *y)
{
    int sx = *x, sy = *y;
    *x = interp(k_sy, (int)(sizeof k_sy / sizeof *k_sy), sy);
    *y = interp(k_sx, (int)(sizeof k_sx / sizeof *k_sx), sx);
}

/* What the runtime needs to know about this game (game_profile.h). */
static const LgGameProfile k_profile = {
    .title = "Let's Go Jungle!",
    /* Lindbergh Yellow ran this game at 1360x768, already 16:9. */
    .width = 1360, .height = 768,
    /* The game runs its logic once a frame and left the timing to the
     * cabinet's vsync: 60 frames a second, whatever the display does. */
    .pace_fps = 60,
    /* Its voices play near full scale, a dozen at once in a firefight:
     * peaks reach three times full scale. 0.4 keeps them at about the
     * limiter's knee and the attract music at a normal level. */
    .sound_gain = 0.4f,
    /* The gun goes through gun_map above; nothing linear on top. */
    .aim = { 1, 0, 1, 0 },
    .gun_map = gun_map,
    .vr = {
        .title = "Let's Go Jungle VR",
        /* The world is in about decimetres: the passenger's arms 5 to 7
         * units from the eye, the ground below the jeep 35 along a ray 45
         * degrees down. A unit taken for a metre showed it ten times too
         * big, and aimed from a gun a tenth as far from the head as it is. */
        .scale = 10,
        /* The world is drawn into an 800x600 target and tone mapped onto
         * the screen, and the camera is kept in the vertex programs'
         * constants rather than in GL's matrices (stereo.c). */
        .world_w = 800, .world_h = 600,
        .camera_in_constants = 1,
        /* It leaves the world's target for its shadow and reflection
         * targets before the jungle is drawn: the depth is the world's only
         * after the last world draw. */
        .depth_read = LG_DEPTH_AUTO,
    },
};

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr,
            "usage: %s <lgj_final> [game args...]\n"
            "\n"
            "  lgj_final is Let's Go Jungle's executable, from a game tree you\n"
            "  can already read. None of it ships here - see the README.\n",
            argv[0]);
        return 2;
    }

    /* Before anything guest-shaped runs, so a fault in lifted code reports
     * the simulated machine rather than a C function name in a generated file. */
    guest_install_crash_handler();

    if (guest_load(argv[1], argc - 1, argv + 1) != 0)
        return 1;

    /* The cabinet's launch script points TEA_DIR at the game's folder, where
     * the Cg runtime (hle_cg.c) finds the precompiled shaders. */
    if (!getenv("TEA_DIR")) setenv("TEA_DIR", ".", 1);

    game_profile_apply(&k_profile);

    /* Give the imports bodies. Everything the toolkit ships is bound here;
     * anything this game needs beyond that gets bound on top, and whatever is
     * still missing aborts naming itself on first call. */
    hle_register_all();
    {
        const char *nm = "_ZN9acpBackup23getEEPROM_analog_adjustEv";
        uint32_t va = guest_symbol(nm);
        if (va && (g_analog_adjust = dispatch_lifted(va)) && guest_override(nm, h_analog_adjust))
            fprintf(stderr, "[lgj] gun: a blank calibration reads as the whole range\n");
    }

    CPU cpu;
    guest_init_cpu(&cpu);

    /* The guest never returns here: its exit path is the exit_group syscall,
     * which the runtime turns into the host's exit(). Reaching the line below
     * means the entry point returned, which a Linux _start does not do. */
    dispatch(&cpu, guest_entry());

    fprintf(stderr, "[host] the entry point returned - _start should not.\n");
    return 1;
}
