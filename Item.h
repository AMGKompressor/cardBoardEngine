

enum ItemType {
	Basic,
	Battery,
	Weapon
};

class Player;
class Sprite;
class Renderer;

class Item {
public:
	Item();
	virtual ~Item();
	ItemType getType() const;
	bool Initialise(Renderer& renderer, ItemType type = ItemType::Battery);
	void Draw(Renderer& renderer);
	bool isInPlayerPickupRange(Player& player) const;
	bool tryPickup(Player& player);
	bool tryClickPickup(Player& player, float worldMouseX, float worldMouseY);
	bool trySteal(float thiefX, float thiefY, float radius);
	bool isStolen() const { return mStolen; }
	bool isCollected() const { return mCollected; }
	void setWorldPosition(float x, float y);
	bool containsWorldPoint(float worldX, float worldY) const;

protected:
	void drawBattery(Renderer& renderer) const;
	bool applyPickupEffect(Player& player);

	bool isPickedUp;
	bool mStolen;
	bool mCollected;
	Sprite* m_pSprite;
	ItemType m_type;
	float m_posX;
	float m_posY;
	int m_Value;
	float m_hitboxHalfW;
	float m_hitboxHalfH;

};
