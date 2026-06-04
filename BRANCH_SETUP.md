# Which git branch to use

| Branch | Use for |
|--------|---------|
| **`betaVersionV2-windows`** | **Full game** — Visual Studio `Project1.sln`, enemies, minimap, `game/textures/`, `game/shaders/` |
| `main` | Slimmer CMake demo (`src/`, `framework/`) only |

GitHub’s default branch is **`main`**, so you will **not** see `game/textures/` or most `.cpp` files until you switch:

```bash
git fetch origin
git checkout betaVersionV2-windows
git pull
```

Then confirm:

```bash
dir game\textures
```

You should see `board8x8.png`, `cursor.png`, and `item.png`.

Also copy course **`lib/`** from COMP710 Studio 5 (not in git). See `lib/README.md`.
