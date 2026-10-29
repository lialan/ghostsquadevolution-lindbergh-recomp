/*
 * host.c - run recompiled The House of the Dead 4.
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
#include <string.h>

#include "lindbergh_rt.h"

/* The gun, at the two functions the game's gun code reads it through. Both
 * take the board's raw words and the game's stored calibration; a mouse needs
 * neither, so they answer from the pointer directly.
 *
 * GetAdjstGunPos(&x, &y, rawx, rawy, adjust, ...) -> on screen. The position
 * is a byte each, 0..255 across the whole screen. Register arguments: eax =
 * &x, edx = &y. The pointer outside the window is off the screen. */
static void h_gun_pos(CPU *c)
{
    int mx, my, w, h;
    host_mouse_state(&mx, &my, NULL);
    lindbergh_window_size(&w, &h);
    int on = mx >= 0 && my >= 0 && mx < w && my < h;
    long x = (long)mx * 256 / (w > 0 ? w : 1), y = (long)my * 256 / (h > 0 ? h : 1);
    wr8(c->eax, (uint8_t)(x < 0 ? 0 : x > 255 ? 255 : x));
    wr8(c->edx, (uint8_t)(y < 0 ? 0 : y > 255 ? 255 : y));
    c->eax = on;
}

/* The reload. A House of the Dead 4 gun has an accelerometer and the game
 * reloads when it is shaken. GetAdjstGunAccel(&a, &b, rawa, rawb, adjust)
 * gives the two axes as 0..1 past the calibrated dead zone, and anything over
 * zero is a shake. A mouse cannot shake, so the cabinet's reload input (the
 * right button held, or the pointer leaving the window, which is a single
 * frame) shakes the gun hard for a quarter of a second. Register arguments:
 * eax = &a, edx = &b. */
static void h_gun_accel(CPU *c)
{
    static unsigned left;
    if (host_cabinet_buttons() & CAB_P1_RELOAD) left = 15;
    float v = left ? 1.0f : 0.0f;
    if (left) left--;
    uint32_t bits; memcpy(&bits, &v, 4);
    wr32(c->eax, bits);
    wr32(c->edx, bits);
}

/* GetSpriteResolution(&w, &h) -> screen ratio. The game lays its 2D out for
 * two monitors only: the cabinet's 1280x768 (ratio 2) and 640x480 (every
 * other shape, positions in 640x480 pixels). It then converts those with the
 * sprite resolution, which for a 16:9 monitor is the monitor's own 1280x720,
 * so the HUD came out at half size in the top-left. In a 16:9 mode the sprite
 * resolution is 640x480, matching the layout; the runtime then draws that
 * 4:3 layout at its own shape in the 16:9 frame (host_hud_widen). */
static void h_sprite_res(CPU *c)
{
    wr32(c->eax, 640);
    wr32(c->edx, 480);
    c->eax = 5;                 /* 16:9 */
}

/* whether the HUD keeps to the screen's edges, or stays a centred 4:3 block */
static int hud_anchored(void) { return 1; }

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr,
            "usage: %s <hod4M.elf> [game args...]\n"
            "\n"
            "  hod4M.elf is The House of the Dead 4's executable, from a game tree you\n"
            "  can already read. None of it ships here - see the README.\n",
            argv[0]);
        return 2;
    }

    /* Before anything guest-shaped runs, so a fault in lifted code reports
     * the simulated machine rather than a C function name in a generated file. */
    guest_install_crash_handler();

    if (guest_load(argv[1], argc - 1, argv + 1) != 0)
        return 1;

    /* The monitor the game draws for: the cabinet's 1280x768. The game has a
     * 1280x720 mode too, but only its 3D follows it - the 2D stays laid out
     * for the cabinet - so it draws at 1280x768 and the window scales that to
     * whatever size it is. LINDBERGH_WIDTH/HEIGHT pick another of its modes. */
    host_window_hint(1280, 720);

    /* Give the imports bodies. Everything the toolkit ships is bound here;
     * anything this game needs beyond that gets bound on top, and whatever is
     * still missing aborts naming itself on first call. */
    hle_register_all();
    {
        int w, h;
        lindbergh_window_size(&w, &h);
        if (w * 9 == h * 16 && guest_override("_Z19GetSpriteResolutionPjS_", h_sprite_res)) {
            fprintf(stderr, "[hotd4] 16:9: the 2D keeps its 4:3 layout\n");
            hud_widen_enable(0.75f, hud_anchored);
        }
    }
    if (guest_override("_Z14GetAdjstGunPosPhS_ttP18GunAdjustValueTypei", h_gun_pos))
        fprintf(stderr, "[hotd4] aim: the gun is where the pointer is\n");
    if (guest_override("_Z16GetAdjstGunAccelPfS_ttP23GunAccelAdjustValueType", h_gun_accel))
        fprintf(stderr, "[hotd4] reload: the reload input shakes the gun\n");

    CPU cpu;
    guest_init_cpu(&cpu);

    /* The guest never returns here: its exit path is the exit_group syscall,
     * which the runtime turns into the host's exit(). Reaching the line below
     * means the entry point returned, which a Linux _start does not do. */
    dispatch(&cpu, guest_entry());

    fprintf(stderr, "[host] the entry point returned - _start should not.\n");
    return 1;
}
