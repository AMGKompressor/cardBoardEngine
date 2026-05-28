# Windows / Visual Studio layout

Open **`cardBoardEngine/Project1.vcxproj`** (or `TGP.slnx`) in **Visual Studio 2022** on Windows.

## Before first build

1. Copy COMP710 **`lib/`** (SDL2, SDL2_image, GLEW only) into `cardBoardEngine/lib/` — see `cardBoardEngine/lib/README.md`.
2. Use platform **Win32** (same as the course framework).
3. Build — output goes to `cardBoardEngine/game/` (exe + copy `SDL2.dll`, `SDL2_image.dll`, `glew32.dll` there if the post-build step does not).

## Not in git (you recreate locally)

- `lib/SDL*`, `lib/glew*`, `lib/FMOD`, `lib/imgui`
- `temp/`, `out/`, `game/*.exe`, `game/*.dll`, `game/*.pdb`
- `*.vcxproj.user`
