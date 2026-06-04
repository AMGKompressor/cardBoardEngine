#include "Item.h"
#include "Renderer.h"
#include "Player_1.h"
#include "Sprite.h"


Item::Item()
	:isPickedUp {false}
	, m_pSprite {0}
	, m_type {Basic}
	, m_posX{0.0f}
	, m_posY{0.0f}
	, m_Value {0}
{

}

Item::~Item() {
	delete m_pSprite;
	m_pSprite = 0;
}

bool Item::Initialise(Renderer& renderer) {
	m_pSprite = renderer.createSprite("../assets/textures/item.png");
	m_posX = 500.0f;
	m_posY = 800.0f;
	m_pSprite->setScale(5);
	m_pSprite->setX(m_posX);
	m_pSprite->setY(m_posY);
	return true;
}

void Item::Draw(Renderer& renderer) {
	if (m_pSprite != 0) {
		m_pSprite->draw(renderer);
	}
}

ItemType Item::getType() {
	return m_type;
}

bool Item::isInPlayerPickupRange(Player& player) {
	return true;
}