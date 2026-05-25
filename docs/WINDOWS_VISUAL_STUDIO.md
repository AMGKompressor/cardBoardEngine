# Windows: fixing LNK2019 / “unresolved external symbol” errors

If you see errors like these, **stop** — you are almost certainly **not** building cardBoardEngine the way this repo expects:

- `unresolved external symbol LogManager::getInstance`
- `unresolved external symbol Renderer::Initialise` or `DrawSprite`
- `unresolved external symbol AnimatedSprite::Draw`
- `unresolved external symbol Game::PreloadSprites`
- Project name **TGP**, path like `TGP - Phantom Raider\Project1`

Those mean your **Phantom Raider / TGP Visual Studio solution** was mixed with cardBoard **without** linking the full `framework/` from this repo (and/or you still have the **old course renderer** with different function names).

cardBoardEngine uses **`initialize`**, **`draw`**, **`drawSprite`** (lowercase). It does **not** include `Game`, `AnimatedSprite`, or `PreloadSprites`.

---

## Recommended fix (everyone should do this)

**Do not** copy `.cpp` files into an existing TGP `.vcxproj` until you know what you are doing.

1. **Clone only this repo:**

   ```bat
   git clone https://github.com/AMGKompressor/cardBoardEngine.git
   cd cardBoardEngine
   ```

2. **Copy COMP710 `lib/`** into `cardBoardEngine\lib\` (see `lib\README.md`).

3. **Build with CMake (Win32):**

   ```bat
   cd cardBoardEngine
   cmake -G "Visual Studio 17 2022" -A Win32 -B build
   cmake --build build --config Release
   ```

4. **Run:**

   ```bat
   cd build\Release
   cardboard.exe
   ```

   Put `SDL2.dll`, `SDL2_image.dll`, `glew32.dll` next to `cardboard.exe` (from course `lib`).

You should get **`cardboard.exe`**, not **`TGP_debug.exe`**.

---

## Why your errors happen (short version)

| Error mentions | Meaning |
|----------------|---------|
| `LogManager::getInstance` | `framework\logmanager.cpp` **not** added to your VS project |
| `Sprite::draw` / `TextureManager::initialize` | `sprite.cpp`, `texturemanager.cpp`, etc. **missing** from linker |
| `Renderer::Initialise`, `DrawSprite` | Your **old TGP headers** — not cardBoard’s `renderer.h` |
| `AnimatedSprite`, `Game::PreloadSprites` | Your **game code** — not part of cardBoardEngine |

Linker errors (LNK2019) = **.h file included, matching .cpp not compiled into the exe**.

---

## Only if you must merge into Phantom Raider / TGP (.sln)

Use **cardBoard’s** `framework\` files, not duplicate renderer code from Studio 5 with different names.

### A. Add these `.cpp` files to the project (Required)

```
framework\logmanager.cpp
framework\matrix4.cpp
framework\renderer.cpp
framework\shader.cpp
framework\sprite.cpp
framework\texture.cpp
framework\texturemanager.cpp
framework\vertexarray.cpp
src\main.cpp
src\CardBoardApp.cpp
src\Collision2D.cpp
src\Map\Map.cpp
src\Player\Player.cpp
```

### B. Include directories (Project → Properties → C/C++ → General → Additional Include Directories)

```
path\to\cardBoardEngine\framework
path\to\cardBoardEngine\src
path\to\cardBoardEngine\src\Map
path\to\cardBoardEngine\src\Player
```

Plus SDL/GLEW includes from `lib\` (same as your other TGP project).

### C. Do **not** compile both renderers

- Remove **or** exclude from build: old `Renderer.cpp` / `Sprite.cpp` from TGP if they define `Initialise`, `DrawSprite`, `AnimatedSprite`.
- Remove **or** exclude: `Game.cpp` pieces that call `PreloadSprites` unless you implement them.

cardBoard’s demo entry is **`src\main.cpp`** → `CardBoardApp`, not `Game::initialise`.

### D. Link libraries

Same as COMP710 Win32: SDL2, SDL2main, SDL2_image, glew32, opengl32.

### E. Post-build: copy assets

Copy `assets\shaders` and `assets\textures` next to your `.exe`, or set working directory to a folder that contains `shaders\` and `textures\`.

---

## Still broken?

1. Confirm you can build **stock** cardBoardEngine with CMake first (`cardboard.exe` runs).
2. Then integrate **one folder at a time** (framework → Map → Player → App).
3. Do not mix British-spelling API (`Initialise`) headers with cardBoard `.cpp` files.

If CMake build works but TGP does not, the problem is **project setup**, not the engine source on GitHub.
