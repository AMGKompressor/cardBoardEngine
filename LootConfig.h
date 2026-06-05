#pragma once

// =============================================================================
// LootConfig.h — loot tiers, economy, and ART SWAP POINTS for sprites
// =============================================================================
//
// WORLD PICKUPS (floor):
//   1. Add PNGs under assets/textures/ (names below).
//   2. Set kLootUseSprites = true.
//   3. Tune lootWorldSpriteScale() if needed.
//
// INVENTORY HUD (bottom slots):
//   1. Same PNGs (or separate loot_inv_*.png — set paths in lootInventorySpritePath).
//   2. Set kInventoryLootUseSprites = true.
//   3. See UI.cpp ::drawInventorySlotIcon().
//
// Until sprites exist, colored procedural triangles are drawn (Item.cpp / UI.cpp).
// =============================================================================

enum class LootTier : int
{
	Grey = 0,    // low value
	Orange = 1,  // low–mid
	Blue = 2,    // mid
	Purple = 3,  // high
	Count = 4
};

struct LootTierInfo
{
	float r = 0.3f;
	float g = 0.3f;
	float b = 0.3f;
	int value = 0;
	const char* label = "";
};

inline LootTierInfo lootTierInfo(LootTier tier)
{
	switch (tier)
	{
	case LootTier::Grey:
		return { 0.22f, 0.24f, 0.28f, 75, "Scrap" };
	case LootTier::Orange:
		return { 1.0f, 0.52f, 0.08f, 280, "Bronze" };
	case LootTier::Blue:
		return { 0.25f, 0.55f, 0.95f, 550, "Sapphire" };
	case LootTier::Purple:
		return { 0.62f, 0.28f, 0.92f, 950, "Relic" };
	default:
		return { 0.5f, 0.5f, 0.5f, 0, "?" };
	}
}

// --- Sprite toggle (flip when PNGs are in assets/textures/) -----------------

inline constexpr bool kLootUseSprites = true;
inline constexpr bool kInventoryLootUseSprites = true;

// World pickup texture paths (loaded by Item::loadLootSprite).
inline const char* lootWorldSpritePath(LootTier tier)
{
	switch (tier)
	{
	case LootTier::Grey:
		return "/assets/textures/item_common.png";
	case LootTier::Orange:
		return "/assets/textures/item_uncommon.png";
	case LootTier::Blue:
		return "/assets/textures/item_rare.png";
	case LootTier::Purple:
		return "/assets/textures/item_exotic.png";
	default:
		return "/assets/textures/cursor.png";
	}
}

// Inventory slot icons (can match world art or use smaller variants).
inline const char* lootInventorySpritePath(LootTier tier)
{
	switch (tier)
	{
	case LootTier::Grey:
		return "/assets/textures/item_common.png";
	case LootTier::Orange:
		return "/assets/textures/item_uncommon.png";
	case LootTier::Blue:
		return "/assets/textures/item_rare.png";
	case LootTier::Purple:
		return "/assets/textures/item_exotic.png";
	default:
		return "/assets/textures/cursor.png";
	}
}

// Scale passed to Sprite::setScale (same units as player board8x8).
inline float lootWorldSpriteScale(LootTier tier)
{
	(void)tier;
	return 0.5f;
}

inline float lootInventorySpriteScale(LootTier tier)
{
	(void)tier;
	return 0.5f;
}

// Procedural placeholder size on floor (only used when kLootUseSprites is false).
inline float lootPlaceholderHalfWidth()
{
	return 20.0f;
}

inline float lootPlaceholderHeight()
{
	return 34.0f;
}

inline constexpr int kExtractionGoalDollars = 1000;

inline float extractionProgressTowardZero(int moneyCollected, int extractionGoal)
{
	if (extractionGoal <= 0)
	{
		return 1.0f;
	}
	const float t = static_cast<float>(moneyCollected) / static_cast<float>(extractionGoal);
	if (t < 0.0f)
	{
		return 0.0f;
	}
	if (t > 1.0f)
	{
		return 1.0f;
	}
	return t;
}

inline void extractionMoneyColor(float progressTowardZero, float& outR, float& outG, float& outB)
{
	float t = progressTowardZero;
	if (t < 0.0f)
	{
		t = 0.0f;
	}
	if (t > 1.0f)
	{
		t = 1.0f;
	}

	struct ColorStop
	{
		float t;
		float r;
		float g;
		float b;
	};

	static const ColorStop stops[] = {
		{ 0.00f, 0.95f, 0.12f, 0.12f },
		{ 0.28f, 1.00f, 0.38f, 0.05f },
		{ 0.52f, 1.00f, 0.62f, 0.08f },
		{ 0.76f, 0.98f, 0.82f, 0.12f },
		{ 0.92f, 0.75f, 0.88f, 0.18f },
		{ 1.00f, 0.18f, 0.95f, 0.28f },
	};

	const int stopCount = static_cast<int>(sizeof(stops) / sizeof(stops[0]));
	if (t <= stops[0].t)
	{
		outR = stops[0].r;
		outG = stops[0].g;
		outB = stops[0].b;
		return;
	}

	for (int i = 1; i < stopCount; ++i)
	{
		if (t <= stops[i].t)
		{
			const float t0 = stops[i - 1].t;
			const float t1 = stops[i].t;
			const float u = (t1 > t0) ? (t - t0) / (t1 - t0) : 1.0f;
			outR = stops[i - 1].r + (stops[i].r - stops[i - 1].r) * u;
			outG = stops[i - 1].g + (stops[i].g - stops[i - 1].g) * u;
			outB = stops[i - 1].b + (stops[i].b - stops[i - 1].b) * u;
			return;
		}
	}

	outR = stops[stopCount - 1].r;
	outG = stops[stopCount - 1].g;
	outB = stops[stopCount - 1].b;
}
