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

Run these from **this project folder** (`cardBoardEngineWINDOWS`).

Install dependencies once (Homebrew):

```bash
brew install cmake sdl2 sdl2_image glew pkg-config
```

Build the **full Visual Studio game** (`tgp`, same entry as `main.cpp` + `Game.cpp`):

```bash
cd path/to/cardBoardEngineWINDOWS
cmake -B build
cmake --build build
./build/tgp
```

Optional: build only the slimmer modular engine demo (`cardboard`):

```bash
cmake -B build -DCARDBOARD_BUILD_FULL_GAME=OFF
cmake --build build
./build/cardboard
```

Shaders and textures are copied next to the executable automatically. Audio on Mac requires the FMOD library; without it the game still runs (no sound).

## Windows — (Build and Run)

1. Install **Visual Studio 2022** (Desktop C++) and **CMake**.
2. Copy course **`lib/`** into `cardBoardEngine/lib/` (see `lib/README.md`).
3. Configure **Win32**:

```bat
cd path\to\cardBoardEngine
cmake -G "Visual Studio 17 2022" -A Win32 -B build
cmake --build build --config Release
```

4. Run from the output folder with DLLs present:

```bat
cd build\Release
cardboard.exe
```

Put `SDL2.dll`, `SDL2_image.dll`, and `glew32.dll` next to `cardboard.exe` (from the course `lib` folders).

### Visual Studio solution (`Project1.sln`)

Use branch **`betaVersionV2-windows`** (not `main`). Open `Project1.sln`, set configuration to **Debug | Win32** or **Release | Win32** (course `lib` is x86). Build once — a post-build step copies `assets/textures/` and `game/shaders/` into `game/` next to the `.exe`. Run **`game/Project1_debug.exe`** (Debug) or **`game/Project1.exe`** (Release). If textures still fail, confirm `game/textures/board8x8.png` exists after build.

## Controls

- **WASD** — move  
- **Shift** — sprint  
- **LMB** — toggle flashlight  
- **H** — debug hitbox + footstep pulse rings  
- **Esc** — quit  

## Tuning

- `src/Player/PlayerConfig.h` — speeds, flashlight beam, footsteps  
- `src/Map/BasicMapLayout.h` — wall segments  
