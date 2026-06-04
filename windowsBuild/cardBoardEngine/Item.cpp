#include "Item.h"
#include "Renderer.h"
#include "Player_1.h"
#include "Sprite.h"
#include "logmanager.h"

#include <SDL.h>

#include <cmath>

namespace
{
	float distSq(float ax, float ay, float bx, float by)
	{
		const float dx = bx - ax;
		const float dy = by - ay;
		return dx * dx + dy * dy;
	}

	void drawGreyTriangleCap(
		Renderer& renderer,
		float tipX,
		float tipY,
		float baseX,
		float baseHalfH,
		bool tipPointsPositiveX)
	{
		const float topY = tipY - baseHalfH;
		const float botY = tipY + baseHalfH;

		float xy[6] = {
			baseX, topY,
			baseX, botY,
			tipX, tipY
		};
		renderer.drawWorldLineLoop(xy, 3, 0.30f, 0.32f, 0.36f, 1.0f);

		const float capCenterX = (baseX + tipX) * 0.5f;
		const float capHalfW = std::fabs(tipX - baseX) * 0.5f;
		renderer.drawWorldAxisAlignedQuad(
			capCenterX,
			tipY,
			capHalfW,
			baseHalfH * 0.92f,
			0.36f,
			0.38f,
			0.42f,
			0.98f);
	}
}

Item::Item()
	:isPickedUp {false}
	, mStolen {false}
	, mCollected {false}
	, m_pSprite {0}
	, m_type {ItemType::Battery}
	, m_posX{0.0f}
	, m_posY{0.0f}
	, m_Value {0}
	, m_hitboxHalfW{32.0f}
	, m_hitboxHalfH{18.0f}
{

}

Item::~Item() {
	delete m_pSprite;
	m_pSprite = 0;
}

void Item::setWorldPosition(float x, float y) {
	m_posX = x;
	m_posY = y;
	if (m_pSprite != nullptr)
	{
		m_pSprite->setX(m_posX);
		m_pSprite->setY(m_posY);
	}
}

bool Item::Initialise(Renderer& renderer, ItemType type) {
	m_type = type;
	mStolen = false;
	mCollected = false;
	isPickedUp = false;

	if (m_type == ItemType::Basic)
	{
		m_pSprite = renderer.createSprite("textures/item.png");
		if (m_pSprite == nullptr)
		{
			return false;
		}
		m_pSprite->setScale(5);
		m_pSprite->setX(m_posX);
		m_pSprite->setY(m_posY);
		m_hitboxHalfW = 40.0f;
		m_hitboxHalfH = 40.0f;
		return true;
	}

	(void)renderer;
	m_hitboxHalfW = 32.0f;
	m_hitboxHalfH = 18.0f;
	return true;
}

// Battery pickup: LMB in range refills flashlight (see tryClickPickup).
void Item::drawBattery(Renderer& renderer) const
{
	const float bodyHalfW = 26.0f;
	const float bodyHalfH = 12.0f;
	const float blackHalfW = bodyHalfW * 0.75f;
	const float orangeHalfW = bodyHalfW * 0.25f;

	const float bodyLeft = m_posX - bodyHalfW;
	const float bodyRight = m_posX + bodyHalfW;
	const float blackCenterX = bodyLeft + blackHalfW;
	const float orangeCenterX = bodyRight - orangeHalfW;

	const float capHalfH = bodyHalfH * 0.55f;
	const float capTipLen = 5.0f;

	// Thin bright green outline hugging the battery (pulses)
	const float outlineHalfW = bodyHalfW + capTipLen + 3.0f;
	const float outlineHalfH = bodyHalfH + 3.0f;
	const float pulseT = static_cast<float>(SDL_GetTicks()) * 0.001f;
	const float pulse = 0.5f + 0.5f * std::sin(pulseT * 4.5f);
	const float pulseAlpha = 0.70f + 0.30f * pulse;
	const float pulsePad = 1.5f + pulse * 1.5f;

	const float hw = outlineHalfW + pulsePad;
	const float hh = outlineHalfH + pulsePad * 0.5f;
	const float outlineVerts[8] = {
		m_posX - hw, m_posY - hh,
		m_posX + hw, m_posY - hh,
		m_posX + hw, m_posY + hh,
		m_posX - hw, m_posY + hh
	};
	renderer.drawWorldLineLoop(
		outlineVerts,
		4,
		0.35f,
		1.0f,
		0.55f,
		0.75f * pulseAlpha
		);

	renderer.drawWorldAxisAlignedQuad(
		blackCenterX,
		m_posY,
		blackHalfW,
		bodyHalfH,
		0.06f,
		0.06f,
		0.06f,
		1.0f);

	renderer.drawWorldAxisAlignedQuad(
		orangeCenterX,
		m_posY,
		orangeHalfW,
		bodyHalfH,
		1.0f,
		0.52f,
		0.08f,
		1.0f);

	// Grey triangle caps flush with body ends (no gap)
	drawGreyTriangleCap(
		renderer,
		bodyRight + capTipLen,
		m_posY,
		bodyRight,
		capHalfH,
		true);

	drawGreyTriangleCap(
		renderer,
		bodyLeft - capTipLen,
		m_posY,
		bodyLeft,
		capHalfH,
		false);
}

void Item::Draw(Renderer& renderer) {
	if (mStolen || mCollected)
	{
		return;
	}

	if (m_type == ItemType::Battery)
	{
		drawBattery(renderer);
		return;
	}

	if (m_pSprite != 0)
	{
		m_pSprite->draw(renderer);
	}
}

ItemType Item::getType() const {
	return m_type;
}

bool Item::containsWorldPoint(float worldX, float worldY) const
{
	if (mStolen || mCollected)
	{
		return false;
	}

	return std::fabs(worldX - m_posX) <= m_hitboxHalfW
		&& std::fabs(worldY - m_posY) <= m_hitboxHalfH;
}

bool Item::isInPlayerPickupRange(Player& player) const {
	if (mStolen || mCollected)
	{
		return false;
	}
	const float pickupRadius = 100.0f;
	return distSq(m_posX, m_posY, player.x(), player.y()) <= pickupRadius * pickupRadius;
}

bool Item::applyPickupEffect(Player& player)
{
	if (m_type == ItemType::Battery)
	{
		player.refillFlashlightCharge();
		LogManager::getInstance().log("Picked up battery — flashlight charge restored to 100%.");
		return true;
	}

	return true;
}

bool Item::tryPickup(Player& player) {
	if (!isInPlayerPickupRange(player))
	{
		return false;
	}

	mCollected = true;
	isPickedUp = true;
	applyPickupEffect(player);
	return true;
}

bool Item::tryClickPickup(Player& player, float worldMouseX, float worldMouseY)
{
	if (!containsWorldPoint(worldMouseX, worldMouseY))
	{
		return false;
	}

	const float clickPickupRadius = 140.0f;
	if (distSq(m_posX, m_posY, player.x(), player.y()) > clickPickupRadius * clickPickupRadius)
	{
		return false;
	}

	mCollected = true;
	isPickedUp = true;
	applyPickupEffect(player);
	return true;
}

bool Item::trySteal(float thiefX, float thiefY, float radius) {
	if (mStolen || mCollected)
	{
		return false;
	}

	if (distSq(thiefX, thiefY, m_posX, m_posY) > radius * radius)
	{
		return false;
	}

	mStolen = true;
	m_posX += 400.0f;
	m_posY -= 200.0f;
	return true;
}
