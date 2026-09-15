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

## Status — lifts and boots, faults in the game's own allocator

| | |
|---|---:|
| Functions lifted | 6,033 |
| Instructions | 379,487 |
| Coverage | **99.975%** |
| Unlifted | 90, all port I/O or data read as code |
| Builds | yes, clean, first try |
| Reaches `main` | yes |
| Renders | not yet |

It gets through CRT start up, static constructors, into `main`, through nine
GLUT calls, and into its heap manager — where it writes to `0x0000000c`.

What that is **not**, each ruled out by measurement rather than argument:

* **Not GLUT.** The game has not reached rendering. Every GLUT call so far is a
  permissive no-op and none of them is on this path.
* **Not an empty heap table.** The descriptor table is static in `.rodata`,
  five entries, and four are live. A `base` of zero there does not mean failure
  — it means *allocate it yourself*, and the code falls through to `malloc`.
* **Not a failed allocation.** `LINDBERGH_ALLOC=1` shows all four heaps getting
  real memory: 256K, 640K, 256K and 27MB, every one a valid pointer.
* **Not `rep stosd` clobbering flags.** The compiler schedules `test edx, edx`
  two instructions ahead of the branch that uses it, so a string op that wrote
  flags would break this and every other game. It does not; the lifted form
  touches no flags.

So the fault is inside the game's own `mh_malloc_internal`, walking a structure
`init_heap_memory` has just built, with a valid heap pointer in `ecx`.

### What is in the way of finding out

The crash report gives registers and an `ebp` backtrace but **not the faulting
guest instruction**, which is why the four theories above were each argued from
disassembly rather than settled in a minute. Mapping the host fault address
back to a guest address belongs in the toolkit, and pays for itself on every
game after this one.

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
