# ghostsquadevolution-lindbergh-recomp

**Ghost Squad Evolution (Sega, 2007) — a Lindbergh light-gun shooter,
statically recompiled from its x86 ELF to native C.**

Built on [**lindberghrecomp**](https://github.com/sp00nznet/lindberghrecomp),
the Sega Lindbergh recompilation toolkit, vendored here as a git submodule.

> **No game data here.** No ELF, no disc, no filesystem image, no keys. The
> `.gitignore` refuses all of it — including the game binary itself, which has
> no extension and so slipped past an extension list once already. Bring a
> game tree you can already read; the recompiled C is output you generate.

## Why this one

It is the cleanest target on the platform, which is not obvious until you read
what it links against.

Most of the work on *Let's Go Jungle* went into things this game does not have.
It wants no Cg, no `XF86VidMode`, and — remarkably for a Sega arcade title —
**none of SEGA's own libraries**. No `libsegaapi`, no `amLib`, so no base
board, no security dongle, no JVS packet protocol standing between it and its
first frame. Its entire platform surface is GLUT, GL/GLU, X11, pthread, libc,
ncurses and zlib.

It is also the smallest: 6,033 functions against Let's Go Jungle's 31,759.

The one thing it does need that the toolkit has never had is **GLUT**. That is
not a small detail — GLUT inverts control. A game registers callbacks and hands
its loop to the toolkit, where *Let's Go Jungle* drives its own loop and calls
swap when it pleases. This game calls `glutMainLoop`, which never returns, so
the runtime's handler has to *become* the loop and call back into lifted code.

Three of the Lindbergh titles checked so far are GLUT-based and only two are
not, so that seam is worth building once and is why this game is next.

## Status — boots, renders, takes a coin, dies in the I/O loop

![Ghost Squad Evolution, its own resolution, one credit in](docs/credit.png)

| | |
|---|---|
| Lifts | 6,033 functions, 379,487 instructions, **99.975%** |
| Builds | clean, first try |
| Window and GL context | yes, at the 640x480 the game asks for |
| GLUT loop and callbacks | yes, all twelve |
| Boot sequence | developer logo, coin, start, SEGA warning screen |
| Input | coin, start, trigger and gun all reach the game |
| Runs indefinitely | **no** — dies after a few hundred frames |

It gets through CRT start up, its own heap manager, GLUT setup, and into its
boot sequence. A coin registers (`CREDIT(S) 1`), start is accepted, and it
reaches the SEGA warning screen. Then, several hundred frames later, the
process disappears.

### What the failure is, and is not

The exit code is `0xC0000409` — a Windows fail-fast. That is raised through
`__fastfail`, which the kernel handles directly: no vectored exception
handler runs, no `atexit` hook runs, and nothing is printed. From the outside
it is indistinguishable from the process being switched off.

Ruled out, each by measurement rather than argument:

| | how |
|---|---|
| the game calling `exit()` | bound and logged; never fires |
| an unlifted instruction | `RECOMP_TODO` reports and exits 44; never fires |
| a CRT invalid parameter | handler installed, and **proven to fire** on a different fault |
| `abort()` | `_CALL_REPORTFAULT` cleared so it becomes a catchable `SIGABRT`; never fires |
| a pure virtual call, or an exception unwinding | both bound to report; neither fires |
| the analog report | `LINDBERGH_NO_GUN=1` pins the gun centred; no change |
| `qsort` writing out of bounds | logged: 13 elements of 4 bytes, and bounded anyway |
| stubbed library objects overlapping | each symbol now gets its own 256 KB; no change |
| a different JVS packet layout to the first game | `amJvspClearPacket` is byte-identical in both |
| a different backup wrapper argument order | identical in both |
| stale object files after a header change | full clean rebuild; no change |

What is left is host-side memory corruption: something writes over the host's
own stack, and `/GS` notices at the next return. The dispatch ring - which is
flushed to disc precisely because this failure leaves nothing behind - puts
the last calls in the JVS polling loop both with and without a coin inserted,
which is where the runtime's own code runs most often rather than necessarily
where the fault is.

The honest next step is a debugger. None is installed here, and every
in-process approach has now been tried.

## Reproducing it

```powershell
git clone --recursive https://github.com/sp00nznet/ghostsquadevolution-lindbergh-recomp
cd ghostsquadevolution-lindbergh-recomp

# what the game asks the board for
py -3.11 -m tools elf path\to\vsg                 # run from lindberghrecomp\

# lift it
py -3.11 -m tools recomp path\to\vsg ..\games\ghostsquadevolution\generated

# build - 32-bit, and the CMake will not let you forget
cmake -S . -B build -A Win32
cmake --build build --config Release
```

The cabinet's own launcher runs the game from inside its directory, with the
OpenAL driver on the library path and no `TEA_DIR`:

```powershell
cd path\to\disk1\vsg_l
..\..\build\Release\ghostsquadevolution.exe vsg
```

Runtime switches are documented in
[the toolkit](https://github.com/sp00nznet/lindberghrecomp/blob/main/docs/instruments.md).

## Layout

```
lindberghrecomp/                    the toolkit (git submodule)
games/ghostsquadevolution/
  src/host.c                        load the ELF, enter it, get out of the way
  generated/                        the lifted C (you generate it; gitignored)
```

## Legal

The host and the recompilation toolchain are original work, MIT-licensed.
**No game data, no disc images, no keys, and no circumvention of anything.**
*Ghost Squad Evolution* is © Sega; this is an independent, non-commercial
preservation project. Built on
[**lindberghrecomp**](https://github.com/sp00nznet/lindberghrecomp).
