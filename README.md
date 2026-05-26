# cardBoard Engine

Standalone C++ engine — **not part of Boxu**. Clone this repo only; add `lib/` from COMP710 Studio 5 (see `lib/README.md`).

Modular layout:

| Module | Folder | Role |
|--------|--------|------|
| Framework | `framework/` | OpenGL renderer, sprites, shaders |
| Player | `src/Player/` | Movement, sprint, flashlight, hitbox, footstep pulses |
| Map | `src/Map/` | Walls, collision, floor draw |
| App | `src/CardBoardApp.*` | Game loop, input, camera |

## What you need before building

1. **CMake** + C++17 compiler (Xcode CLT on Mac, Visual Studio 2022 on Windows).
2. **`lib/`** — copy SDL2, SDL2_image, GLEW from **COMP710 Studio 5** into `lib/` (see `lib/README.md`).

## Build & run (Mac)

**Important:** run these from the **`cardBoardEngine`** folder itself — not from `Boxu/temp/build-boxu`.

```bash
cd /Users/bardiakhalifeh/Downloads/comp710Game/GameProject23196422/cardBoardEngine
cmake -B build
cmake --build build
./build/cardboard
```

Run from the build folder (shaders/textures are copied next to the exe automatically).

## Windows — (Build and Run)

> **LNK2019 errors in `TGP` / `Phantom Raider` / `TGP_debug.exe`?**  
> You merged files into the wrong Visual Studio project. Use **CMake below** first, or read **[docs/WINDOWS_VISUAL_STUDIO.md](docs/WINDOWS_VISUAL_STUDIO.md)**.  
> Do **not** mix this engine with the old course `Initialise` / `DrawSprite` / `AnimatedSprite` API.

1. Install **Visual Studio** (2019, 2022, or newer) with **Desktop development with C++**, plus **CMake**.
2. Copy course **`lib/`** into `cardBoardEngine/lib/` (see `lib/README.md`).
3. Configure **Win32** — use the generator name for **your** Visual Studio version:

```bat
cd path\to\cardBoardEngine
cmake -G
```

That prints available generators. Pick the newest **Visual Studio …** entry (examples):

| Visual Studio installed | CMake `-G` name (examples) |
|-------------------------|----------------------------|
| 2022 | `"Visual Studio 17 2022"` |
| 2025 / newer | `"Visual Studio 18 2025"` (or whatever `cmake -G` shows) |

If unsure, run `cmake -G` with no other args, copy the exact name, then:

```bat
cmake -G "Visual Studio 18 2025" -A Win32 -B build
cmake --build build --config Release
```

**Newer VS is fine** — you do not need to downgrade to 2022. Still use **`-A Win32`** (32-bit) for the course `lib/` layout.

4. Run from the output folder with DLLs present:

```bat
cd build\Release
cardboard.exe
```

Put `SDL2.dll`, `SDL2_image.dll`, and `glew32.dll` next to `cardboard.exe` (from the course `lib` folders).

### Pre-built Windows zip for teammates

Do **not** push multi‑GB zips to git (GitHub limit **100 MB** per file).

Package only `cardboard.exe`, the three DLLs, `shaders\`, and `textures\` — see **[docs/WINDOWS_RELEASE_ZIP.md](docs/WINDOWS_RELEASE_ZIP.md)**. Upload that small zip via **GitHub Releases** or file share.

## Controls

- **WASD** — move  
- **Shift** — sprint  
- **LMB** — toggle flashlight  
- **H** — debug hitbox + footstep pulse rings  
- **Esc** — quit  

## Tuning

- `src/Player/PlayerConfig.h` — speeds, flashlight beam, footsteps  
- `src/Map/BasicMapLayout.h` — wall segments  
