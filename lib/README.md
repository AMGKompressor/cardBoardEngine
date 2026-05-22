# Third-party libraries (not part of cardBoard source)

cardBoard needs **SDL2**, **SDL2_image**, and **GLEW** — the same COMP710 / Studio 5 bundle used for course games.

## Setup

Copy these three folders from your **COMP710 Studio 5 resources** (or any machine that already has them) into this `lib/` directory:

```
lib/
  SDL2-2.0.22/
  SDL2_image-2.6.1/
  glew-2.1.0/
```

**Do not** copy from the old Boxu game repo if you are not allowed to share that project — use the **official course `lib` zip** instead (same layout, no game code).

### Windows (Win32)

- Use the **x86** `.lib` and `.dll` files inside those folders.
- At run time, put `SDL2.dll`, `SDL2_image.dll`, and `glew32.dll` next to `cardboard.exe` or in your run folder.

### Mac

- Homebrew SDL2 / SDL2_image / GLEW also works; CMake will find them if `lib/` is missing.
