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

## Status — boots, allocates, registers its callbacks, wants a window

| | |
|---|---:|
| Functions lifted | 6,033 |
| Instructions | 379,487 |
| Coverage | **99.975%** |
| Unlifted | 90, all port I/O or data read as code |
| Builds | yes, clean, first try |
| Reaches `main` | yes |
| Survives its heap manager | yes |
| Registers all 12 GLUT callbacks | yes |
| Renders | not yet — no window, no GL context |

It now runs through CRT start up, static constructors, `main`, its own heap
manager, all twelve GLUT callback registrations, and into graphics
initialisation, where it stops:

```
[gl] glGenProgramsARB unavailable on this driver
[gl] glBindProgramARB unavailable on this driver
[gl] glProgramStringARB unavailable on this driver
[hle] __assert_fail
```

Those entry points are not missing from the driver. There is no GL context to
resolve them against, because every GLUT call so far is a permissive no-op.
The game checks, finds nothing, and asserts. **GLUT is now genuinely the next
thing in the way**, which it was not before.

### The allocator bug, and why it took four wrong theories

Getting here meant finding a write to `0x0000000c` inside the game's own
`mh_malloc_internal`. It was none of the things it looked like: not GLUT, not
an empty heap table, not a failed allocation, not `rep stosd` clobbering the
flags of a `test` scheduled two instructions ahead of its branch.

glibc hands any request at or above its mmap threshold — 128 KiB by default —
straight to mmap, so a large block always comes back page aligned. Guest code
never asks for that and never checks it, which is exactly why it can depend on
it without anyone noticing. This game rounds its first free block up to a
32-byte boundary: on a page-aligned heap that is a no-op, but on the merely
16-aligned block the runtime used to return it shifts the block by 16 bytes,
and the split-off block then overlaps the header of the block it was split
from. The allocator reads a `next` pointer out of memory it has just zeroed,
and writes through the null it finds — two functions away from the cause, with
nothing pointing back at malloc.

What settled it was not more reading. It was taking the register dump and the
faulting address from the crash report and evaluating every memory operand in
the function against them. Exactly one resolved to `0xC`, and its base
register was a pointer loaded from sixteen bytes inside a block that had just
been zeroed. That is a minute's work and it should have been the first move.

Fixed in the toolkit: allocations at or above the mmap threshold are page
aligned, as glibc's are.

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
