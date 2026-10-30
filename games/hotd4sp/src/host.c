/*
 * host.c - run recompiled The House of the Dead 4 Special.
 *
 * The same engine as The House of the Dead 4, rebuilt with gcc and shipped
 * stripped: tools bounds finds its functions, and names.txt names the few the
 * runtime looks up. Everything else works as games/hotd4/src/host.c does.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "game_profile.h"
#include "lindbergh_rt.h"

/* The gun, where the game reads it: GetAdjstGunPos(&x, &y, rawx, rawy,
 * adjust, ...) turns the board's raw words into a byte each, 0..255 across
 * the screen, through the stored calibration. A mouse needs no calibration,
 * so this answers from the pointer, as games/hotd4 does. Register arguments:
 * eax = &x, edx = &y. Outside the window the gun is off the screen. */
static void h_gun_pos(CPU *c)
{
    int mx, my, w, h;
    host_mouse_state(&mx, &my, NULL);
    lindbergh_window_size(&w, &h);
    int on = mx >= 0 && my >= 0 && mx < w && my < h;
    long x = ((long)mx * 255 + (w - 1) / 2) / (w > 1 ? w - 1 : 1),
         y = ((long)my * 255 + (h - 1) / 2) / (h > 1 ? h - 1 : 1);
    wr8(c->eax, (uint8_t)(x < 0 ? 0 : x > 255 ? 255 : x));
    wr8(c->edx, (uint8_t)(y < 0 ? 0 : y > 255 ? 255 : y));
    c->eax = on;
}

/* The keychip's second check. Starting a game, the Special has the keychip
 * decrypt a 1 KB block of its own (0x084BA7E0) under a fresh random IV and
 * compares the result with the plaintext it carries (0x084BABE0); a match is
 * a licensed board, anything else is "Error 15 - Game Program Not Found". The
 * cryptography lives in the keychip, so for that one request this answers
 * what a genuine one would, and fails anything else as no keychip does.
 * Arguments: iv, iv, in, out, length. */
#define KEY_CHALLENGE  0x084ba7e0u
#define KEY_EXPECTED   0x084babe0u
#define KEY_CRYPT      0x08363fd7u      /* the keychip library's block decrypt */
static void h_key_crypt(CPU *c)
{
    if (A32(2) == KEY_CHALLENGE && A32(4) == 0x400) {
        memcpy(APTR(3), GP(KEY_EXPECTED), 0x400);
        RET(0);
        return;
    }
    RET(-5);
}

/* LINDBERGH_GOD=1: the player takes no damage, for test runs that must reach
 * a later scene. CPlayer::CanDamagePlayer() - matched against the original
 * game's, the same code with the player's fields moved - answers no. */
#define CAN_DAMAGE_PLAYER 0x080c18d4u
static void h_no_damage(CPU *c) { RET(0); }

/* What the runtime needs to know about this game (game_profile.h). */
static const LgGameProfile k_profile = {
    .title = "The House of the Dead 4 Special",
    .width = 1360, .height = 768,
    .aim = { 1, 0, 1, 0 },
    /* the seats: turned, shaken and blown at by a board on the first serial port */
    .ride_board = "/dev/ttyS0",
    /* its language follows the region in the EEPROM's system data */
    .sysdata_region = 1,
    /* a shake of the gun (its reload) is the second switch byte's bit 6 */
    .reload_switch = { 1, 0x40 },
    .vr = {
        .title = "The House of the Dead 4 Special VR",
        .width = 640, .height = 480,
        .scale = 1,
        .depth_read = LG_DEPTH_AUTO,
        .gl_thread = 1,         /* the street: 32-37 fps with the GL on the game's thread, 56-58 without */
        .hud_head_locked = 1,   /* the life bar and prompts where the eye expects them, wherever it looks */
    },
};

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr,
            "usage: %s <hod4M.elf> [game args...]\n"
            "\n"
            "  hod4M.elf is The House of the Dead 4 Special's executable, from a game\n"
            "  tree you can already read. None of it ships here - see the README.\n",
            argv[0]);
        return 2;
    }

    guest_install_crash_handler();
    if (guest_load(argv[1], argc - 1, argv + 1) != 0)
        return 1;
    game_profile_apply(&k_profile);
    hle_register_all();
    guest_override_va("keychip decrypt", KEY_CRYPT, h_key_crypt);
    {
        const char *g = getenv("LINDBERGH_GOD");
        if (g && *g == '1' && guest_override_va("CPlayer::CanDamagePlayer", CAN_DAMAGE_PLAYER, h_no_damage))
            fprintf(stderr, "[hotd4sp] the player takes no damage (LINDBERGH_GOD)\n");
    }
    if (guest_override("_Z14GetAdjstGunPosPhS_ttP18GunAdjustValueTypei", h_gun_pos))
        fprintf(stderr, "[hotd4sp] aim: the gun is where the pointer is\n");

    CPU cpu;
    guest_init_cpu(&cpu);
    dispatch(&cpu, guest_entry());

    fprintf(stderr, "[host] the entry point returned - _start should not.\n");
    return 1;
}
