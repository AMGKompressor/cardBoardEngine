// =============================================================================
// Item.cpp — world pickup visuals
//
// LOOT ART SWAP:
//   1. Drop PNGs in assets/textures/ (paths in LootConfig.h ::lootWorldSpritePath).
//   2. Set LootConfig::kLootUseSprites = true.
//   3. loadLootSprite() + drawLootVisual() handle the rest.
//
// PLACEHOLDER (current):
//   drawLootPlaceholder() draws a colored floor triangle per LootTierInfo.
// =============================================================================

#include "Item.h"
#include "LootConfig.h"
#include "Renderer.h"
#include "Player_1.h"
#include "Sprite.h"
#include "logmanager.h"
#include "SoundSystem.h"

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

	// Procedural floor triangle (remove when kLootUseSprites is true).
	void drawFloorLootTriangle(
		Renderer& renderer,
		float centerX,
		float centerY,
		float halfWidth,
		float height,
		float r,
		float g,
		float b)
	{
		const float baseY = centerY + height * 0.38f;
		const float tipY = centerY - height * 0.62f;

		const float outer[6] = {
			centerX - halfWidth, baseY,
			centerX + halfWidth, baseY,
			centerX, tipY
		};
		renderer.drawWorldLineLoop(outer, 3, r * 0.7f, g * 0.7f, b * 0.7f, 1.0f);

		const float fillScale = 0.72f;
		const float innerHalfW = halfWidth * fillScale;
		const float innerBaseY = centerY + height * 0.30f;
		const float innerTipY = centerY - height * 0.48f;
		const float inner[6] = {
			centerX - innerHalfW, innerBaseY,
			centerX + innerHalfW, innerBaseY,
			centerX, innerTipY
		};
		renderer.drawWorldLineLoop(inner, 3, r, g, b, 0.95f);

		renderer.drawWorldAxisAlignedQuad(
			centerX,
			centerY + height * 0.05f,
			halfWidth * 0.42f,
			height * 0.22f,
			r,
			g,
			b,
			0.55f);
	}

	void drawColoredTriangle(
		Renderer& renderer,
		float tipX,
		float tipY,
		float baseX,
		float baseHalfH,
		float r,
		float g,
		float b,
		float outlineA,
		float fillA)
	{
		const float topY = tipY - baseHalfH;
		const float botY = tipY + baseHalfH;

		float xy[6] = {
			baseX, topY,
			baseX, botY,
			tipX, tipY
		};
		renderer.drawWorldLineLoop(xy, 3, r * 0.85f, g * 0.85f, b * 0.85f, outlineA);

		const float capCenterX = (baseX + tipX) * 0.5f;
		const float capHalfW = std::fabs(tipX - baseX) * 0.5f;
		renderer.drawWorldAxisAlignedQuad(
			capCenterX,
			tipY,
			capHalfW,
			baseHalfH * 0.92f,
			r,
			g,
			b,
			fillA);
	}
}

Item::Item()
	:isPickedUp {false}
	, mStolen {false}
	, mCollected {false}
	, m_pSprite {0}
	, m_type {ItemType::Battery}
	, m_lootTier {LootTier::Grey}
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

LootTier Item::lootTier() const
{
	return m_lootTier;
}

int Item::lootValue() const
{
	return m_Value;
}

bool Item::loadLootSprite(Renderer& renderer)
{
	delete m_pSprite;
	m_pSprite = nullptr;

	if (!kLootUseSprites)
	{
		return false;
	}

	m_pSprite = renderer.createSprite(lootWorldSpritePath(m_lootTier));
	if (m_pSprite == nullptr)
	{
		return false;
	}

	m_pSprite->setScale(lootWorldSpriteScale(m_lootTier));
	m_pSprite->setX(m_posX);
	m_pSprite->setY(m_posY);
	return true;
}

bool Item::initialiseLoot(Renderer& renderer, LootTier tier)
{
	m_type = ItemType::Loot;
	m_lootTier = tier;
	mStolen = false;
	mCollected = false;
	isPickedUp = false;

	const LootTierInfo info = lootTierInfo(tier);
	m_Value = info.value;
	m_hitboxHalfW = 22.0f;
	m_hitboxHalfH = 20.0f;

	// Try sprite first; placeholder used in drawLootVisual() if this fails.
	loadLootSprite(renderer);
	return true;
}

bool Item::Initialise(Renderer& renderer, ItemType type) {
	m_type = type;
	mStolen = false;
	mCollected = false;
	isPickedUp = false;

	if (m_type == ItemType::Loot)
	{
		return initialiseLoot(renderer, m_lootTier);
	}

	if (m_type == ItemType::Basic)
	{
		// Generic pickup sprite — replace path here or add ItemType-specific paths.
		m_pSprite = renderer.createSprite("/assets/textures/battery.png");
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

	m_pSprite = renderer.createSprite("/assets/textures/battery.png");
	if (m_pSprite == nullptr)
	{
		return false;
	}
	m_pSprite->setScale(0.8f);
	m_pSprite->setX(m_posX);
	m_pSprite->setY(m_posY);
	m_hitboxHalfW = 32.0f;
	m_hitboxHalfH = 18.0f;
	return true;
}

void Item::drawLootPlaceholder(Renderer& renderer) const
{
	const LootTierInfo info = lootTierInfo(m_lootTier);
	drawFloorLootTriangle(
		renderer,
		m_posX,
		m_posY,
		lootPlaceholderHalfWidth(),
		lootPlaceholderHeight(),
		info.r,
		info.g,
		info.b);
}

void Item::drawLootVisual(Renderer& renderer) const
{
	if (m_pSprite != nullptr)
	{
		m_pSprite->setX(m_posX);
		m_pSprite->setY(m_posY);
		m_pSprite->draw(renderer);
		return;
	}

	drawLootPlaceholder(renderer);
}

void Item::drawBattery(Renderer& renderer) const
{
	// Battery uses procedural art. To use a sprite instead:
	//   m_pSprite = renderer.createSprite("/assets/textures/battery.png");
	//   then draw m_pSprite in Draw() and skip drawBattery().
	if (m_pSprite != nullptr)
	{
		m_pSprite->setX(m_posX);
		m_pSprite->setY(m_posY);
		m_pSprite->draw(renderer);
	}/*
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
		0.75f * pulseAlpha,
		1.0f);

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

	drawColoredTriangle(
		renderer,
		bodyRight + capTipLen,
		m_posY,
		bodyRight,
		capHalfH,
		0.36f,
		0.38f,
		0.42f,
		1.0f,
		0.98f);

	drawColoredTriangle(
		renderer,
		bodyLeft - capTipLen,
		m_posY,
		bodyLeft,
		capHalfH,
		0.30f,
		0.32f,
		0.36f,
		1.0f,
		0.98f);*/
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

	if (m_type == ItemType::Loot)
	{
		drawLootVisual(renderer);
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
		SoundSystem::GetInstance().PlaySound("battery_replenish");
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
