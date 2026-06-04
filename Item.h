#include "LootConfig.h"

// =============================================================================
// Item.h — world pickups (loot triangles, batteries, future weapons)
//
// Drawing:
//   Loot  → Item::drawLootVisual() uses sprite if kLootUseSprites else placeholder.
//   Battery → procedural drawBattery() (swap to sprite same pattern as loot).
//   Basic → m_pSprite from /assets/textures/item.png
//
// To add loot PNGs: see LootConfig.h, then Item::loadLootSprite().
// =============================================================================

enum ItemType {
	Basic,
	Battery,
	Weapon,
	Loot
};

class Player;
class Sprite;
class Renderer;

class Item {
public:
	Item();
	virtual ~Item();
	ItemType getType() const;
	LootTier lootTier() const;
	int lootValue() const;

	// Battery / Basic: type-driven. Loot: call initialiseLoot() instead.
	bool Initialise(Renderer& renderer, ItemType type = ItemType::Battery);
	bool initialiseLoot(Renderer& renderer, LootTier tier);

	void Draw(Renderer& renderer);
	bool isInPlayerPickupRange(Player& player) const;
	bool tryPickup(Player& player);
	bool tryClickPickup(Player& player, float worldMouseX, float worldMouseY);
	bool trySteal(float thiefX, float thiefY, float radius);
	bool isStolen() const { return mStolen; }
	bool isCollected() const { return mCollected; }
	void setWorldPosition(float x, float y);
	float worldX() const { return m_posX; }
	float worldY() const { return m_posY; }
	bool containsWorldPoint(float worldX, float worldY) const;

	// True when a loot sprite loaded successfully (for inventory HUD mirroring).
	bool hasLootSprite() const { return m_type == ItemType::Loot && m_pSprite != nullptr; }

protected:
	// --- Loot visuals (sprite OR procedural placeholder) ---------------------
	bool loadLootSprite(Renderer& renderer);
	void drawLootVisual(Renderer& renderer) const;
	void drawLootPlaceholder(Renderer& renderer) const;

	// --- Other item visuals --------------------------------------------------
	void drawBattery(Renderer& renderer) const;

	bool applyPickupEffect(Player& player);

	bool isPickedUp;
	bool mStolen;
	bool mCollected;
	Sprite* m_pSprite;
	ItemType m_type;
	LootTier m_lootTier;
	float m_posX;
	float m_posY;
	int m_Value;
	float m_hitboxHalfW;
	float m_hitboxHalfH;

};
