# Windows release zip (for teammates)

Do **not** commit large `.zip` files to git. GitHub rejects files over **100 MB** (and repos get huge).

## What to zip (small “playable” folder)

After a successful CMake build, zip **only** something like:

```
cardBoard-Windows-Release/
  cardboard.exe          ← from build\Release\
  SDL2.dll
  SDL2_image.dll
  glew32.dll             ← from course lib (Win32 bin folder)
  shaders\               ← copy from assets\shaders
  textures\              ← copy from assets\textures (board8x8.png)
```

Typical size: **a few MB**, not gigabytes.

## What NOT to put in the zip

- Entire repo with `.git\`
- `lib\` FMOD, Qt, docs
- `TGP_debug.exe`, `build\` intermediates
- Other team projects (`game\` folder from Phantom Raider)

## Share the zip

1. **GitHub Release** (recommended): Repo → Releases → New release → attach the small zip.
2. **Discord / Drive / OneDrive** for the team.
3. **Not** `git add windowsBuild.zip` on the main branch.
