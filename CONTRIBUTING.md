# Contributing to ghostsquadevolution-lindbergh-recomp

Anything that gets *Ghost Squad Evolution* closer to playable is welcome. Most of the
work is in the toolkit, so look at
[lindberghrecomp's CONTRIBUTING](https://github.com/sp00nznet/lindberghrecomp/blob/main/CONTRIBUTING.md)
first.

## Where it goes

- **A fix that would help any Lindbergh game** (a libc call, a GL path, the
  JVS board) goes to [lindberghrecomp](https://github.com/sp00nznet/lindberghrecomp),
  not here. This repo then bumps the submodule.
- **Something only this game needs** (an address, a value, a quirk of its
  engine) goes here, in `games/ghostsquadevolution/`.

## Ground rules

- **No game data, keys or lifted source.** That means no ELF, no disc, no
  `generated/` output, no shaders from the disc, and no dumps attached to
  issues. A PR that adds any of these will be closed.
- **Say what you ran it on.** Which dump, what you saw, and the log lines. A
  screenshot is only evidence if it came from a real run.
- **Leave a check behind** for non-trivial logic, the smallest thing that fails
  when it breaks.

## Where your code comes from

This repo is MIT, and it stays MIT only if every line in it is original or
under a licence compatible with MIT. **Don't port code from a GPL emulator or
loader**, and don't write "our own" version with its source open beside you.
That is derivation, and it would make this repo GPL. Running an emulator as a
black box and comparing output is fine. So are hardware docs and measuring the
game itself. Say in the PR where anything non-obvious came from.

## Checks

CI builds and runs the toolkit self-check on every PR. If you have the game,
also lift it, build it and run to the SEGA warning screen (README, Getting Started (Windows), or Quick start (macOS)), and
say in the PR that you did.
