Primary sprite folder for cardBoard (tracked in git).

Core files (in use today):
  board8x8.png — player / enemy placeholder
  cursor.png   — mouse cursor
  item.png     — generic pickup (ItemType::Basic)

Loot sprites (optional — flip flags in LootConfig.h):
  World floor pickups (kLootUseSprites):
    loot_scrap.png, loot_bronze.png, loot_sapphire.png, loot_relic.png
  Inventory HUD icons (kInventoryLootUseSprites):
    loot_scrap_inv.png, loot_bronze_inv.png, loot_sapphire_inv.png, loot_relic_inv.png
  You can reuse the same PNG for world + inv if you only add the world filenames.

Paths in code look like /assets/textures/loot_scrap.png.
When the .exe runs from game/, the engine resolves ../assets/textures/ automatically.

Where to edit:
  LootConfig.h     — filenames, kLootUseSprites, kInventoryLootUseSprites, scales
  Item.cpp         — world pickup draw (loadLootSprite / drawLootVisual)
  UI.cpp           — inventory slot icons (loadInventoryLootSprites)

Until PNGs exist, colored procedural triangles are drawn as placeholders.
