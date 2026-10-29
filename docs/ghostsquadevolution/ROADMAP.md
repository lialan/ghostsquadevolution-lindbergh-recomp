# Roadmap

## Next

- **The fail-fast after the warning screen.** Host-side stack corruption,
  caught by `/GS`. Every in-process approach has been tried, so the next step
  is a debugger (WinDbg, with a breakpoint on the `/GS` failure).
- **Sound.** None yet. The cabinet launcher puts an OpenAL driver on the
  library path, so start by finding out how the game reaches it.
- **A one-click `Setup.cmd`** that checks the prerequisites, asks for the game
  tree, lifts, builds, and leaves a launcher. It also puts the game on
  recomp-launcher.
- **Headless recording** (`--headless --record out.mp4`), once the toolkit has
  it.

## Out of scope

- Decrypting the disc, or distributing any game data or lifted source.
