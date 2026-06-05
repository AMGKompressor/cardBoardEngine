// =============================================================================
// UI.cpp — HUD meters, inventory slots, extraction bar
//
// INVENTORY LOOT ICON SWAP:
//   1. Add PNGs (lootInventorySpritePath in LootConfig.h).
//   2. Set kInventoryLootUseSprites = true.
//   3. Icons load in UI::loadInventoryLootSprites(); drawn in drawInventorySlotIcon().
//
// Until then, drawInventoryLootPlaceholder() draws mini colored triangles.
// =============================================================================

#include "UI.h"
#include "Player_1.h"
#include "PlayerConfig_1.h"
#include "Inventory.h"
#include "LootConfig.h"
#include "Sprite.h"
#include "SoundSystem.h"
#include "renderer.h"

#include "imgui.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <random>

namespace
{
	struct InventoryHudLayout
	{
		float slotW = 72.0f;
		float slotH = 72.0f;
		float gap = 14.0f;
		float panelLeft = 0.0f;
		float panelTop = 0.0f;
		float panelW = 0.0f;
		float panelH = 0.0f;

		float slotCenterX(int index) const
		{
			const float left = panelLeft + 12.0f + static_cast<float>(index) * (slotW + gap);
			return left + slotW * 0.5f;
		}

		float slotCenterY() const
		{
			return panelTop + 12.0f + slotH * 0.5f;
		}

		float keyLabelWorldY() const
		{
			return panelTop + 12.0f + slotH + 10.0f;
		}
	};

	InventoryHudLayout buildInventoryHudLayout(
		float cameraX,
		float cameraY,
		float viewWidth,
		float viewHeight)
	{
		InventoryHudLayout layout;
		layout.panelW = layout.slotW * static_cast<float>(Inventory::kSlotCount)
			+ layout.gap * static_cast<float>(Inventory::kSlotCount - 1) + 24.0f;
		layout.panelH = layout.slotH + 24.0f;
		layout.panelLeft = cameraX + viewWidth * 0.5f - layout.panelW * 0.5f;
		layout.panelTop = cameraY + viewHeight - layout.panelH - 40.0f;
		return layout;
	}

	// Procedural mini-triangle in an inventory slot (placeholder art).
	void drawInventoryLootPlaceholder(
		Renderer& renderer,
		float cx,
		float cy,
		const LootTierInfo& info)
	{
		const float triHalf = 18.0f;
		const float verts[6] = {
			cx - triHalf, cy + triHalf * 0.6f,
			cx + triHalf, cy + triHalf * 0.6f,
			cx, cy - triHalf * 0.9f
		};
		renderer.drawWorldLineLoop(verts, 3, info.r, info.g, info.b, 0.95f);
		renderer.drawWorldAxisAlignedQuad(
			cx,
			cy,
			triHalf * 0.45f,
			triHalf * 0.45f,
			info.r,
			info.g,
			info.b,
			0.85f);
	}

	// One occupied slot: sprite if enabled and loaded, else placeholder triangle.
	void drawInventorySlotIcon(
		Renderer& renderer,
		float cx,
		float cy,
		LootTier tier,
		Sprite* const* lootSprites,
		int lootSpriteCount)
	{
		if (kInventoryLootUseSprites && lootSprites != nullptr)
		{
			const int index = static_cast<int>(tier);
			if (index >= 0 && index < lootSpriteCount && lootSprites[index] != nullptr)
			{
				lootSprites[index]->setX(cx);
				lootSprites[index]->setY(cy);
				lootSprites[index]->draw(renderer);
				return;
			}
		}

		drawInventoryLootPlaceholder(renderer, cx, cy, lootTierInfo(tier));
	}
}

UI::UI()
{
	for (int i = 0; i < kLootInventorySpriteSlots; ++i)
	{
		m_lootInventorySprites[i] = nullptr;
	}
}

UI::~UI()
{
	clearInventoryLootSprites();
	mConfig = NULL;
	ui_player = NULL;
}

void UI::loadInventoryLootSprites(Renderer& renderer)
{
	clearInventoryLootSprites();

	if (!kInventoryLootUseSprites)
	{
		return;
	}

	for (int i = 0; i < kLootInventorySpriteSlots; ++i)
	{
		const LootTier tier = static_cast<LootTier>(i);
		m_lootInventorySprites[i] = renderer.createSprite(lootInventorySpritePath(tier));
		if (m_lootInventorySprites[i] != nullptr)
		{
			m_lootInventorySprites[i]->setScale(lootInventorySpriteScale(tier));
		}
	}
}

void UI::clearInventoryLootSprites()
{
	for (int i = 0; i < kLootInventorySpriteSlots; ++i)
	{
		delete m_lootInventorySprites[i];
		m_lootInventorySprites[i] = nullptr;
	}
}

bool UI::initialise(Renderer& renderer, Player* player, PlayerConfig* config)
{
	std::srand(std::time(NULL));
	mConfig = config;
	ui_player = player;
	loadInventoryLootSprites(renderer);

	return true;
}

void UI::process(float deltaTime)
{

}

void UI::draw(Renderer& renderer, float cameraX, float cameraY)
{
	if (ui_player->getDarkStatus() || ui_player->getSanityPercentage() <= 9.0f)
	{
		drawSanityMeter(renderer, cameraX, cameraY);
	}

	if (ui_player->isRunning == true || ui_player->staminaPercentage <= 85.0f)
	{
		drawStaminaMeter(renderer, cameraX, cameraY);
	}

	drawHealthMeter(renderer, cameraX, cameraY);
}

void UI::drawStaminaMeter(Renderer& renderer, float cameraX, float cameraY)
{
	const StaminaMeterConfig& meter = mConfig->staminaMeter;
	if (meter.width <= 2.0f || meter.height <= 2.0f)
	{
		return;
	}

	const float left = ui_player->mX - (meter.width * 0.5f);
	const float top = ui_player->mY - 90.0f;

	const float centerX = left + meter.width * 0.5f;
	const float centerY = top + meter.height * 0.5f;

	renderer.drawWorldAxisAlignedQuad(
		centerX,
		centerY,
		meter.width * 0.5f,
		meter.height * 0.5f,
		0.05f,
		0.05f,
		0.05f,
		0.85f);

	const float ratio = staminaRatio();
	const float innerHeight = std::max(1.0f, meter.height - meter.border * 2.0f);
	const float innerWidthMax = std::max(1.0f, meter.width - meter.border * 2.0f);
	const float innerWidth = std::max(1.0f, innerWidthMax * ratio);
	const float innerLeft = left + meter.border;
	const float innerCenterX = innerLeft + innerWidth * 0.5f;
	const float innerCenterY = top + meter.height * 0.5f;

	const float red = 0.90f;
	const float green = 0.90f;
	const float blue = 0.10f;

	renderer.drawWorldAxisAlignedQuad(
		innerCenterX,
		innerCenterY,
		innerWidth * 0.5f,
		innerHeight * 0.5f,
		red,
		green,
		blue,
		0.95f);
}

void UI::adjustStamina(float deltaTime)
{
	if (ui_player->isRunning == true)
	{
		ui_player->staminaPercentage = std::max(
			0.0f,
			ui_player->staminaPercentage -
			mConfig->staminaMeter.drainPerSecond * deltaTime);
	}
	else
	{
		ui_player->staminaPercentage = std::min(
			mConfig->staminaMeter.maxStamina,
			ui_player->staminaPercentage +
			mConfig->staminaMeter.rechargePerSecond * deltaTime);
	}
}

float UI::staminaRatio()
{
	const float maxStamina = mConfig->staminaMeter.maxStamina;
	if (maxStamina <= 0.0f)
	{
		return 0.0f;
	}


	return std::min(1.0f, std::max(0.0f, ui_player->staminaPercentage / maxStamina));
}

void UI::drawSanityMeter(Renderer& renderer, float cameraX, float cameraY)
{
	const SanityMeterConfig& meter = mConfig->sanityMeter;
	if (meter.width <= 2.0f || meter.height <= 2.0f)
	{
		return;
	}

	const float left = ui_player->mX - (meter.width * 0.5f);
	const float top = ui_player->mY - 80.0f;

	const float centerX = left + meter.width * 0.5f;
	const float centerY = top + meter.height * 0.5f;

	renderer.drawWorldAxisAlignedQuad(
		centerX,
		centerY,
		meter.width * 0.5f,
		meter.height * 0.5f,
		0.05f,
		0.05f,
		0.05f,
		0.85f);

	const float ratio = sanityRatio();
	const float innerHeight = std::max(1.0f, meter.height - meter.border * 2.0f);
	const float innerWidthMax = std::max(1.0f, meter.width - meter.border * 2.0f);
	const float innerWidth = std::max(1.0f, innerWidthMax * ratio);
	const float innerLeft = left + meter.border;
	const float innerCenterX = innerLeft + innerWidth * 0.5f;
	const float innerCenterY = top + meter.height * 0.5f;

	const float red = (ratio < 0.35f) ? 0.95f : 0.20f;
	const float green = (ratio < 0.35f) ? 0.25f : 0.90f;
	const float blue = 0.20f;

	renderer.drawWorldAxisAlignedQuad(
		innerCenterX,
		innerCenterY,
		innerWidth * 0.5f,
		innerHeight * 0.5f,
		red,
		green,
		blue,
		0.95f);
}
bool UI::noSanity(float deltaTime)
{
	if (ui_player->sanityPercentage <= 0.1f)
	{
		SoundSystem::GetInstance().PlaySound("low_sanity");
		playRandomThrowIn();
		adjustHealth(deltaTime);
		return true;
	}
	return false;
}

void UI::adjustSanity(float deltaTime)
{
	const float enemyDrain = ui_player->enemySanityDrainPerSecond;
	const float darkDrain = ui_player->inDark ? mConfig->sanityMeter.drainPerSecond : 0.0f;
	const float totalDrain = enemyDrain + darkDrain;

	if (totalDrain > 0.0f)
	{
		ui_player->sanityPercentage = std::max(
			0.0f,
			ui_player->sanityPercentage - totalDrain * deltaTime);
	}
	else if (!ui_player->inDark)
	{
		ui_player->sanityPercentage = std::min(
			mConfig->sanityMeter.maxCharge,
			ui_player->sanityPercentage +
			mConfig->sanityMeter.rechargePerSecond * deltaTime);
	}

	noSanity(deltaTime);
}

float UI::sanityRatio()
{
	const float maxCharge = mConfig->sanityMeter.maxCharge;
	if (maxCharge <= 0.0f)
	{
		return 0.0f;
	}


	return std::min(1.0f, std::max(0.0f, ui_player->sanityPercentage / maxCharge));

}

void UI::drawHealthMeter(Renderer& renderer, float cameraX, float cameraY)
{
	const HealthMeterConfig& meter = mConfig->healthMeter;
	if (meter.width <= 2.0f || meter.height <= 2.0f)
	{
		return;
	}

	const float left = ui_player->mX - (meter.width * 0.5f);
	const float top = ui_player->mY - 100.0f;

	const float centerX = left + meter.width * 0.5f;
	const float centerY = top + meter.height * 0.5f;

	renderer.drawWorldAxisAlignedQuad(
		centerX,
		centerY,
		meter.width * 0.5f,
		meter.height * 0.5f,
		0.05f,
		0.05f,
		0.05f,
		0.85f);

	const float ratio = healthRatio();
	const float innerHeight = std::max(1.0f, meter.height - meter.border * 2.0f);
	const float innerWidthMax = std::max(1.0f, meter.width - meter.border * 2.0f);
	const float innerWidth = std::max(1.0f, innerWidthMax * ratio);
	const float innerLeft = left + meter.border;
	const float innerCenterX = innerLeft + innerWidth * 0.5f;
	const float innerCenterY = top + meter.height * 0.5f;

	const float red = (ratio < 0.35f) ? 0.95f : 0.20f;
	const float green = (ratio < 0.35f) ? 0.25f : 0.90f;
	const float blue = 0.20f;

	renderer.drawWorldAxisAlignedQuad(
		innerCenterX,
		innerCenterY,
		innerWidth * 0.5f,
		innerHeight * 0.5f,
		red,
		green,
		blue,
		0.95f);
}

void UI::adjustHealth(float deltaTime)
{
	ui_player->playerHealth = std::max(
		0.0f,
		ui_player->playerHealth -
		mConfig->healthMeter.decay * deltaTime);
}

float UI::healthRatio()
{
	const float maxCharge = mConfig->healthMeter.maxCharge;
	if (maxCharge <= 0.0f)
	{
		return 0.0f;
	}

	return std::min(1.0f, std::max(0.0f, ui_player->playerHealth / maxCharge));
}

void UI::drawInventoryHud(
	Renderer& renderer,
	float cameraX,
	float cameraY,
	float viewWidth,
	float viewHeight,
	const Inventory& inventory) const
{
	const InventoryHudLayout layout =
		buildInventoryHudLayout(cameraX, cameraY, viewWidth, viewHeight);

	renderer.drawWorldAxisAlignedQuad(
		layout.panelLeft + layout.panelW * 0.5f,
		layout.panelTop + layout.panelH * 0.5f,
		layout.panelW * 0.5f,
		layout.panelH * 0.5f,
		0.12f,
		0.12f,
		0.14f,
		0.55f);

	for (int i = 0; i < Inventory::kSlotCount; ++i)
	{
		const float cx = layout.slotCenterX(i);
		const float cy = layout.slotCenterY();
		const bool selected = (i == inventory.selectedSlot());

		if (selected)
		{
			const float pad = 5.0f;
			const float halfW = layout.slotW * 0.5f + pad;
			const float halfH = layout.slotH * 0.5f + pad;
			renderer.drawWorldAxisAlignedQuad(
				cx,
				cy,
				halfW,
				halfH,
				0.58f,
				0.60f,
				0.65f,
				0.88f);

			const float outline[8] = {
				cx - halfW, cy - halfH,
				cx + halfW, cy - halfH,
				cx + halfW, cy + halfH,
				cx - halfW, cy + halfH
			};
			renderer.drawWorldLineLoop(outline, 4, 0.72f, 0.74f, 0.80f, 0.95f);
		}

		renderer.drawWorldAxisAlignedQuad(
			cx,
			cy,
			layout.slotW * 0.5f,
			layout.slotH * 0.5f,
			0.22f,
			0.22f,
			0.24f,
			selected ? 0.58f : 0.45f);

		if (inventory.slotOccupied(i))
		{
			const InventorySlot& slot = inventory.slot(i);
			drawInventorySlotIcon(
				renderer,
				cx,
				cy,
				slot.tier,
				m_lootInventorySprites,
				kLootInventorySpriteSlots);
		}
		else if (selected)
		{
			renderer.drawWorldAxisAlignedQuad(
				cx,
				cy,
				layout.slotW * 0.22f,
				layout.slotH * 0.22f,
				0.40f,
				0.40f,
				0.44f,
				0.40f);
		}
	}
}

void UI::drawInventoryKeyLabels(
	float cameraX,
	float cameraY,
	float viewWidth,
	float viewHeight,
	const Inventory& inventory) const
{
	const InventoryHudLayout layout =
		buildInventoryHudLayout(cameraX, cameraY, viewWidth, viewHeight);

	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	if (drawList == nullptr)
	{
		return;
	}

	const float labelWorldY = layout.keyLabelWorldY();
	const int selected = inventory.selectedSlot();

	for (int i = 0; i < Inventory::kSlotCount; ++i)
	{
		const char label[2] = { static_cast<char>('1' + i), '\0' };
		const ImVec2 textSize = ImGui::CalcTextSize(label);
		const float worldCenterX = layout.slotCenterX(i);
		const float screenX = (worldCenterX - cameraX) - textSize.x * 0.5f;
		const float screenY = labelWorldY - cameraY;

		const bool isSelected = (i == selected);
		const ImU32 color = isSelected
			? IM_COL32(235, 235, 245, 255)
			: IM_COL32(130, 132, 140, 210);

		drawList->AddText(ImVec2(screenX, screenY), color, label);
	}
}

void UI::drawExtractionHud(
	Renderer& renderer,
	float cameraX,
	float cameraY,
	float viewWidth,
	float viewHeight,
	int moneyCollected,
	int extractionGoal) const
{
	const float panelW = 220.0f;
	const float panelH = 56.0f;
	const float panelLeft = cameraX + viewWidth - panelW - 24.0f;
	const float panelTop = cameraY + viewHeight - panelH - 28.0f;
	const float cx = panelLeft + panelW * 0.5f;
	const float cy = panelTop + panelH * 0.5f;

	renderer.drawWorldAxisAlignedQuad(
		cx,
		cy,
		panelW * 0.5f,
		panelH * 0.5f,
		0.08f,
		0.08f,
		0.10f,
		0.55f);

	const float progress = extractionProgressTowardZero(moneyCollected, extractionGoal);
	float red = 0.95f;
	float green = 0.12f;
	float blue = 0.12f;
	extractionMoneyColor(progress, red, green, blue);

	const float barW = panelW - 28.0f;
	const float barH = 14.0f;
	renderer.drawWorldAxisAlignedQuad(
		panelLeft + 14.0f + barW * 0.5f,
		panelTop + 38.0f,
		barW * 0.5f,
		barH * 0.5f,
		0.15f,
		0.15f,
		0.17f,
		0.9f);

	if (progress > 0.01f)
	{
		const float fillW = std::max(4.0f, barW * progress);
		renderer.drawWorldAxisAlignedQuad(
			panelLeft + 14.0f + fillW * 0.5f,
			panelTop + 38.0f,
			fillW * 0.5f,
			barH * 0.45f,
			red,
			green,
			blue,
			0.95f);
	}

}

void UI::playRandomThrowIn()
{
	int randSound = (rand() % 4) + 1;
	int chance = (rand() % 10000) + 1;

	if (chance <= 5)
	{
		if (randSound == 1)
		{
			SoundSystem::GetInstance().PlaySound("throw_in1");
		} else if (randSound == 2)
		{
			SoundSystem::GetInstance().PlaySound("throw_in2");
		}
		else if (randSound == 3)
		{
			SoundSystem::GetInstance().PlaySound("throw_in3");
		}
		else if (randSound == 4)
		{
			SoundSystem::GetInstance().PlaySound("throw_in4");
		}

	}
}