#pragma once

#ifndef __UI_H
#define __UI_H

// =============================================================================
// UI.h — sanity/health/stamina meters, inventory HUD, extraction money bar
//
// Inventory loot icons: set LootConfig::kInventoryLootUseSprites and add PNGs
// (see lootInventorySpritePath). Sprites cache in m_lootInventorySprites[].
// =============================================================================

class PlayerConfig;
class Renderer;
class Sprite;
class Player;
class Inventory;

class UI
{
	// MEMBER METHODS
public:
	UI();
	~UI();

	// Inventory loot icons (loaded when LootConfig::kInventoryLootUseSprites is true).
	static constexpr int kLootInventorySpriteSlots = 4;

	bool initialise(Renderer& renderer, Player* player, PlayerConfig* config);
	void process(float deltaTime);
	void draw(Renderer& renderer, float cameraX, float cameraY);
	void drawInventoryHud(
		Renderer& renderer,
		float cameraX,
		float cameraY,
		float viewWidth,
		float viewHeight,
		const Inventory& inventory) const;
	void drawInventoryKeyLabels(
		float cameraX,
		float cameraY,
		float viewWidth,
		float viewHeight,
		const Inventory& inventory) const;
	void drawExtractionHud(
		Renderer& renderer,
		float cameraX,
		float cameraY,
		float viewWidth,
		float viewHeight,
		int moneyCollected,
		int extractionGoal) const;

	// SANITY
	void drawSanityMeter(Renderer& renderer, float cameraX, float cameraY);
	bool noSanity(float deltaTime);
	void adjustSanity(float deltaTime);
	float sanityRatio();

	void playRandomThrowIn();

	// HEALTH
	void drawHealthMeter(Renderer& renderer, float cameraX, float cameraY);
	void adjustHealth(float deltaTime);
	float healthRatio();

	// STAMINA
	void drawStaminaMeter(Renderer& renderer, float cameraX, float cameraY);
	void adjustStamina(float deltaTime);
	float staminaRatio();

protected:
	void loadInventoryLootSprites(Renderer& renderer);
	void clearInventoryLootSprites();

	Sprite* m_lootInventorySprites[kLootInventorySpriteSlots];

private:
	UI(const UI& ui);
	UI& operator=(const UI& ui);

	// MEMBER DATA
public:
	Player* ui_player;
protected:
	PlayerConfig* mConfig;

};

#endif // __UI_H