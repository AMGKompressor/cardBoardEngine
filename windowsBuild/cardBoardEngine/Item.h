

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
	ItemType getType();
	bool Initialise(Renderer& renderer);
	void Draw(Renderer& renderer);
	bool isInPlayerPickupRange(Player& player);
protected:
	bool isPickedUp;
	Sprite* m_pSprite;
	ItemType m_type;
	float m_posX;
	float m_posY;
	int m_Value;

};