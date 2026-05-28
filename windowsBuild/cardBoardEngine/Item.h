

enum ItemType {

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
	void setPosition(float x, float y);
	bool isInPlayerPickupRange(Player& player);
protected:
	Sprite* m_pSprite;
	ItemType m_type;
	float m_posX;
	float m_posY;
	int m_Value;

};