#include "SceneCardBoard.h"

#include "Map_1.h"
#include "ProceduralSpawns.h"
#include "Player_1.h"
#include "PlayerConfig_1.h"

#include "logmanager.h"
#include "renderer.h"
#include "sprite.h"
#include "Item.h"

#include "UI.h"
#include "EnemyManager.h"
#include "Minimap.h"

#include "InputSystem.h"
#include "vector2.h"
#include "game.h"

#include "imgui.h"

#include <SDL.h>

#include "NavGrid.h"
#include "SoundSystem.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace
{
	constexpr const char* kHiddenLowFlashlightSoundKey = "hidden_low_flashlight";
	constexpr float kLowFlashlightSoundInterval = 0.5f;
	constexpr float kLowFlashlightThreshold = 0.50f;

	bool loadHiddenLowFlashlightSound()
	{
		const char* paths[] = {
			"sounds/hiddenSound.mp3",
			"assets/sounds/hiddenSound.mp3",
			"../assets/sounds/hiddenSound.mp3",
		};

		for (const char* path : paths)
		{
			if (SoundSystem::GetInstance().LoadSound(path, kHiddenLowFlashlightSoundKey))
			{
				return true;
			}
		}

		return false;
	}

	bool clampInsideMap(float& x, float& y, const Map& map, float margin)
	{
		const float maxX = map.width() - margin;
		const float maxY = map.height() - margin;
		if (margin >= maxX || margin >= maxY)
		{
			return false;
		}
		x = std::max(margin, std::min(maxX, x));
		y = std::max(margin, std::min(maxY, y));
		return true;
	}

	bool resolveWalkableSpawn(
		float& x,
		float& y,
		const Map& map,
		const NavGrid& nav)
	{
		if (!clampInsideMap(x, y, map, 48.0f))
		{
			return false;
		}

		if (!nav.isBuilt())
		{
			return true;
		}

		if (nav.isWalkableWorld(x, y))
		{
			return true;
		}

		const float step = nav.cellSize();
		for (int ring = 1; ring <= 14; ++ring)
		{
			for (int oy = -ring; oy <= ring; ++oy)
			{
				for (int ox = -ring; ox <= ring; ++ox)
				{
					if (std::abs(ox) != ring && std::abs(oy) != ring)
					{
						continue;
					}

					float tryX = x + static_cast<float>(ox) * step;
					float tryY = y + static_cast<float>(oy) * step;
					if (!clampInsideMap(tryX, tryY, map, 48.0f))
					{
						continue;
					}
					if (nav.isWalkableWorld(tryX, tryY))
					{
						x = tryX;
						y = tryY;
						return true;
					}
				}
			}
		}

		return false;
	}

	// Prefer the walkable cell closest to the player (for Q drops at feet).
	bool resolveWalkableNearPlayer(
		float& x,
		float& y,
		const Map& map,
		const NavGrid& nav,
		float playerX,
		float playerY,
		float maxRadius = 80.0f)
	{
		x = playerX;
		y = playerY;
		if (!clampInsideMap(x, y, map, 40.0f))
		{
			return false;
		}

		if (!nav.isBuilt())
		{
			return true;
		}

		float bestX = x;
		float bestY = y;
		float bestDistSq = maxRadius * maxRadius + 1.0f;
		bool found = false;

		const float kOffsets[][2] = {
			{ 0.0f, 32.0f },
			{ 0.0f, -28.0f },
			{ 30.0f, 18.0f },
			{ -30.0f, 18.0f },
			{ 38.0f, 0.0f },
			{ -38.0f, 0.0f },
			{ 22.0f, 40.0f },
			{ -22.0f, 40.0f },
			{ 0.0f, 0.0f },
		};

		for (const auto& offset : kOffsets)
		{
			float tryX = playerX + offset[0];
			float tryY = playerY + offset[1];
			if (!clampInsideMap(tryX, tryY, map, 40.0f))
			{
				continue;
			}
			if (!nav.isWalkableWorld(tryX, tryY))
			{
				continue;
			}

			const float dx = tryX - playerX;
			const float dy = tryY - playerY;
			const float distSq = dx * dx + dy * dy;
			if (distSq <= bestDistSq)
			{
				bestDistSq = distSq;
				bestX = tryX;
				bestY = tryY;
				found = true;
			}
		}

		if (found)
		{
			x = bestX;
			y = bestY;
			return true;
		}

		return resolveWalkableSpawn(x, y, map, nav);
	}
}

SceneCardBoard::SceneCardBoard()
	: m_pRenderer {0}
	, m_pMap{0}
	, m_pPlayer{0}
	, m_pSceneCardBoard {0}
	, mCameraX {0.0f}
	, mCameraY {0.0f}
	, mLastTime {0}
	, m_pInputSystem{ 0 }
	, mLooping {true}
	, m_pEnemies{0}
	, m_pMinimap{0}
	, mCollectedItems{0}
{

}

SceneCardBoard::~SceneCardBoard() {
	delete m_pPlayer;
	m_pPlayer = nullptr;
	delete m_pMap;
	m_pMap = nullptr;
	clearBatteries();
	clearWorldLoot();

	delete m_pEnemies;
	m_pEnemies = nullptr;

	delete m_pMinimap;
	m_pMinimap = nullptr;

	//Dont delete m_pRenderer since the Game file owns it. we simply remove the pointer to it;
	m_pRenderer = nullptr;

	delete m_pInputSystem;
	m_pInputSystem = nullptr;
}


bool SceneCardBoard::Initialise(Renderer& renderer) {
	m_pRenderer = &renderer;
	
	m_pMap = new Map();
	m_pMap->generate();

	const float spawnX = ProceduralGridLayout::spawnExtractionRoomCenterX();
	const float spawnY = ProceduralGridLayout::spawnExtractionRoomCenterY();
	m_extractionGoal = kExtractionGoalDollars;

	m_pPlayer = new Player();
	m_pPlayerConfig = new PlayerConfig;

	if (!m_pPlayer->initialize(*m_pRenderer, m_pPlayerConfig, spawnX, spawnY))
	{
		LogManager::getInstance().log("cardBoard: player init failed.");
		return false;
	}

	m_pUI = new UI();
	m_pUI->initialise(*m_pRenderer, m_pPlayer, m_pPlayerConfig);

	// Corner HUD map — settings in MinimapConfig.h, drawn in Draw(), ImGui in DebugDraw().
	m_pMinimap = new Minimap();

	m_pEnemies = new EnemyManager();
	m_pEnemies->syncHearingFromPlayerConfig(*m_pPlayerConfig);
	if (!m_pEnemies->buildNavigation(*m_pMap))
	{
		LogManager::getInstance().log("EnemyManager: nav grid build failed.");
		return false;
	}
	if (!m_pEnemies->initialize(*m_pRenderer, spawnX, spawnY))
	{
		LogManager::getInstance().log("EnemyManager failed to init.");
		return false;
	}

	if (!spawnBatteries(renderer))
	{
		LogManager::getInstance().log("Failed to spawn batteries.");
		return false;
	}

	if (!spawnWorldLoot(renderer))
	{
		LogManager::getInstance().log("Failed to spawn loot.");
		return false;
	}

	m_pPlayer->toggleFlashlight();

	mLowFlashlightSoundLoaded = loadHiddenLowFlashlightSound();
	if (!mLowFlashlightSoundLoaded)
	{
		LogManager::getInstance().log(
			"Low-flashlight warning sound not found (expected sounds/hiddenSound.mp3).");
	}

	mLastTime = SDL_GetPerformanceCounter();
	updateCamera();
	m_pRenderer->setCamera(mCameraX, mCameraY);

	LogManager::getInstance().log(
		"cardBoard — random map; spawn room = extraction zone. Collect $1000 loot, return home, press F to extract.");
	return true;
}

void SceneCardBoard::clearBatteries()
{
	for (Item* battery : m_batteries)
	{
		delete battery;
	}
	m_batteries.clear();
}

// Spawn flashlight refill pickups at fixed map positions.
bool SceneCardBoard::spawnBatteries(Renderer& renderer)
{
	clearBatteries();

	const NavGrid* nav = (m_pEnemies != nullptr && m_pEnemies->navGrid().isBuilt())
		? &m_pEnemies->navGrid()
		: nullptr;

	auto spawnBatteryAt = [&](float spawnX, float spawnY) -> bool
	{
		if (m_pMap != nullptr && nav != nullptr)
		{
			resolveWalkableSpawn(spawnX, spawnY, *m_pMap, *nav);
		}

		Item* battery = new Item();
		if (!battery->Initialise(renderer, ItemType::Battery))
		{
			delete battery;
			return false;
		}
		battery->setWorldPosition(spawnX, spawnY);
		m_batteries.push_back(battery);
		return true;
	};

	for (const ProceduralGridLayout::BatterySpawns::Point& spawn
		: ProceduralGridLayout::BatterySpawns::kList)
	{
		if (!spawnBatteryAt(spawn.x, spawn.y))
		{
			clearBatteries();
			return false;
		}
	}

	return true;
}

void SceneCardBoard::clearWorldLoot()
{
	for (Item* loot : m_worldLoot)
	{
		delete loot;
	}
	m_worldLoot.clear();
}

LootTier SceneCardBoard::lootTierForSpawnIndex(int index) const
{
	static const LootTier kTiers[] = {
		LootTier::Grey,
		LootTier::Orange,
		LootTier::Blue,
		LootTier::Purple,
		LootTier::Orange,
		LootTier::Grey,
		LootTier::Blue,
		LootTier::Purple,
	};
	return kTiers[index % static_cast<int>(sizeof(kTiers) / sizeof(kTiers[0]))];
}

bool SceneCardBoard::spawnWorldLoot(Renderer& renderer)
{
	clearWorldLoot();

	if (m_pMap == nullptr || m_pEnemies == nullptr)
	{
		return false;
	}

	const NavGrid& nav = m_pEnemies->navGrid();
	int index = 0;

	auto trySpawnOneLoot = [&](float spawnXIn, float spawnYIn) -> bool
	{
		float spawnX = spawnXIn;
		float spawnY = spawnYIn;
		if (!resolveWalkableSpawn(spawnX, spawnY, *m_pMap, nav))
		{
			LogManager::getInstance().log("Loot spawn skipped (no walkable cell).");
			++index;
			return true;
		}

		Item* loot = new Item();
		if (!loot->initialiseLoot(renderer, lootTierForSpawnIndex(index)))
		{
			delete loot;
			clearWorldLoot();
			return false;
		}
		loot->setWorldPosition(spawnX, spawnY);
		m_worldLoot.push_back(loot);
		++index;
		return true;
	};

	for (const ProceduralGridLayout::LootSpawns::Point& spawn
		: ProceduralGridLayout::LootSpawns::kList)
	{
		if (!trySpawnOneLoot(spawn.x, spawn.y))
		{
			return false;
		}
	}

	return !m_worldLoot.empty();
}

void SceneCardBoard::buildWorldItemList(std::vector<Item*>& outItems) const
{
	outItems.clear();
	for (Item* battery : m_batteries)
	{
		if (battery != nullptr && !battery->isCollected())
		{
			outItems.push_back(battery);
		}
	}
	for (Item* loot : m_worldLoot)
	{
		if (loot != nullptr && !loot->isCollected())
		{
			outItems.push_back(loot);
		}
	}
}

bool SceneCardBoard::tryPickupNearbyLoot()
{
	if (m_pPlayer == nullptr)
	{
		return false;
	}

	Item* closest = nullptr;
	float closestDistSq = 100.0f * 100.0f;

	for (Item* loot : m_worldLoot)
	{
		if (loot == nullptr || loot->isCollected())
		{
			continue;
		}
		if (!loot->isInPlayerPickupRange(*m_pPlayer))
		{
			continue;
		}

		const float dx = loot->worldX() - m_pPlayer->x();
		const float dy = loot->worldY() - m_pPlayer->y();
		const float dSq = dx * dx + dy * dy;
		if (dSq < closestDistSq)
		{
			closestDistSq = dSq;
			closest = loot;
		}
	}

	if (closest == nullptr)
	{
		return false;
	}

	if (!m_inventory.addLoot(closest->lootTier(), closest->lootValue()))
	{
		LogManager::getInstance().log("Inventory full (3 slots).");
		return false;
	}

	m_moneyCollected += closest->lootValue();
	closest->setWorldPosition(-10000.0f, -10000.0f);
	const LootTierInfo info = lootTierInfo(closest->lootTier());
	char msg[128];
	std::snprintf(
		msg,
		sizeof(msg),
		"Picked up %s ($%d). Total: $%d / $%d",
		info.label,
		closest->lootValue(),
		m_moneyCollected,
		m_extractionGoal);
	LogManager::getInstance().log(msg);

	closest->tryPickup(*m_pPlayer);
	return true;
}

bool SceneCardBoard::tryPickupNearbyBattery()
{
	if (m_pPlayer == nullptr)
	{
		return false;
	}

	for (Item* battery : m_batteries)
	{
		if (battery == nullptr || battery->isCollected())
		{
			continue;
		}
		if (battery->tryPickup(*m_pPlayer))
		{
			++mCollectedItems;
			return true;
		}
	}
	return false;
}

bool SceneCardBoard::tryDropSelectedLoot()
{
	if (m_pPlayer == nullptr || m_pRenderer == nullptr)
	{
		return false;
	}

	LootTier tier = LootTier::Grey;
	int value = 0;
	if (!m_inventory.removeSelected(tier, value))
	{
		return false;
	}

	m_moneyCollected = std::max(0, m_moneyCollected - value);

	float dropX = m_pPlayer->x();
	float dropY = m_pPlayer->y();

	const NavGrid& nav = m_pEnemies->navGrid();
	resolveWalkableNearPlayer(dropX, dropY, *m_pMap, nav, m_pPlayer->x(), m_pPlayer->y(), 72.0f);

	Item* loot = new Item();
	if (!loot->initialiseLoot(*m_pRenderer, tier))
	{
		delete loot;
		m_inventory.addLoot(tier, value);
		m_moneyCollected += value;
		return false;
	}
	loot->setWorldPosition(dropX, dropY);
	m_worldLoot.push_back(loot);

	LogManager::getInstance().log("Dropped loot from inventory (Q).");
	return true;
}

void SceneCardBoard::updateLowFlashlightSound(float deltaTime)
{
	if (!mLowFlashlightSoundLoaded || m_pPlayer == nullptr)
	{
		return;
	}

	const float batteryRatio = m_pPlayer->flashlightChargeRatio();
	if (batteryRatio >= kLowFlashlightThreshold)
	{
		mLowFlashlightSoundTimer = 0.0f;
		return;
	}

	mLowFlashlightSoundTimer += deltaTime;
	while (mLowFlashlightSoundTimer >= kLowFlashlightSoundInterval)
	{
		SoundSystem::GetInstance().PlaySound(kHiddenLowFlashlightSoundKey);
		mLowFlashlightSoundTimer -= kLowFlashlightSoundInterval;
	}
}

void SceneCardBoard::updateCamera() {
	if (m_pRenderer == nullptr || m_pPlayer == nullptr || m_pMap == nullptr)
	{
		return;
	}

	const float viewW = static_cast<float>(m_pRenderer->getWidth());
	const float viewH = static_cast<float>(m_pRenderer->getHeight());

	float maxCamX = m_pMap->width() - viewW;
	float maxCamY = m_pMap->height() - viewH;
	if (maxCamX < 0.0f) { maxCamX = 0.0f; }
	if (maxCamY < 0.0f) { maxCamY = 0.0f; }

	mCameraX = m_pPlayer->x() - viewW * 0.5f;
	mCameraY = m_pPlayer->y() - viewH * 0.5f;

	if (mCameraX < 0.0f) { mCameraX = 0.0f; }
	if (mCameraY < 0.0f) { mCameraY = 0.0f; }
	if (mCameraX > maxCamX) { mCameraX = maxCamX; }
	if (mCameraY > maxCamY) { mCameraY = maxCamY; }
}

void SceneCardBoard::Process(float deltaTime, InputSystem& inputSystem) {

	if (inputSystem.GetKeyState(SDL_SCANCODE_ESCAPE) == BS_PRESSED) {
		Game::GetInstance().Quit();
		return;
	}

	if (deltaTime < 0.0f)
	{
		deltaTime = 0.0f;
	}
	else if (deltaTime > 0.1f)
	{
		deltaTime = 0.1f;
	}
	SDL_PumpEvents();
	const Uint8* keys = SDL_GetKeyboardState(nullptr);
	const bool sprintHeld = keys[SDL_SCANCODE_LSHIFT] != 0u || keys[SDL_SCANCODE_RSHIFT] != 0u;
	const bool stunHeld = keys[SDL_SCANCODE_SPACE] != 0u;

	m_pPlayer->update(
		deltaTime,
		*m_pMap,
		mCameraX,
		mCameraY,
		keys[SDL_SCANCODE_W] != 0,
		keys[SDL_SCANCODE_A] != 0,
		keys[SDL_SCANCODE_S] != 0,
		keys[SDL_SCANCODE_D] != 0,
		sprintHeld,
		stunHeld);

	if (inputSystem.GetMouseButtonState(1) == BS_RELEASED)
	{
		m_pPlayer->toggleFlashlight();
	}

	if (inputSystem.GetKeyState(SDL_SCANCODE_1) == BS_PRESSED)
	{
		m_inventory.selectSlot(0);
	}
	if (inputSystem.GetKeyState(SDL_SCANCODE_2) == BS_PRESSED)
	{
		m_inventory.selectSlot(1);
	}
	if (inputSystem.GetKeyState(SDL_SCANCODE_3) == BS_PRESSED)
	{
		m_inventory.selectSlot(2);
	}
	if (inputSystem.GetKeyState(SDL_SCANCODE_E) == BS_PRESSED)
	{
		if (!tryPickupNearbyLoot())
		{
			tryPickupNearbyBattery();
		}
	}
	if (inputSystem.GetKeyState(SDL_SCANCODE_Q) == BS_PRESSED)
	{
		tryDropSelectedLoot();
	}

	if (inputSystem.GetKeyState(SDL_SCANCODE_F) == BS_PRESSED)
	{
		tryExtract();
	}

	if (inputSystem.GetKeyState(SDL_SCANCODE_H) == BS_PRESSED)
	{
		m_pPlayer->setShowHitboxDebug(!m_pPlayer->showHitboxDebug());
		if (m_pPlayer->showHitboxDebug())
		{
			LogManager::getInstance().log(
				"Debug overlay ON (H): hitbox, green nav nodes, yellow enemy BFS paths.");
		}
		else
		{
			LogManager::getInstance().log("Debug overlay OFF (H).");
		}
	}

	if (m_pEnemies != nullptr)
	{
		std::vector<Item*> worldItems;
		buildWorldItemList(worldItems);
		m_pEnemies->update(deltaTime, *m_pMap, *m_pPlayer, worldItems);
	}

	m_pUI->adjustSanity(deltaTime);
	m_pUI->adjustStamina(deltaTime);
	updateLowFlashlightSound(deltaTime);

	updateCamera();
	m_pRenderer->setCamera(mCameraX, mCameraY);
}

void SceneCardBoard::drawExtractionZone() const
{
	if (m_pRenderer == nullptr)
	{
		return;
	}

	const ProceduralGridLayout::RoomBounds room =
		ProceduralGridLayout::spawnExtractionRoomBounds();
	const float cx = (room.left + room.right) * 0.5f;
	const float cy = (room.top + room.bottom) * 0.5f;
	const float inset = 72.0f;
	const float halfW = (room.right - room.left) * 0.5f - inset;
	const float halfH = (room.bottom - room.top) * 0.5f - inset;

	const bool fundsReady = m_moneyCollected >= m_extractionGoal;
	const bool playerInside = m_pPlayer != nullptr
		&& ProceduralGridLayout::isInsideSpawnExtractionRoom(
			m_pPlayer->x(),
			m_pPlayer->y());

	const float baseR = fundsReady ? 0.18f : 0.10f;
	const float baseG = fundsReady ? 0.72f : 0.28f;
	const float baseB = fundsReady ? 0.30f : 0.14f;
	const float alpha = playerInside && fundsReady ? 0.42f : 0.24f;

	m_pRenderer->drawWorldAxisAlignedQuad(
		cx,
		cy,
		halfW,
		halfH,
		baseR,
		baseG,
		baseB,
		alpha);
}

bool SceneCardBoard::tryExtract()
{
	if (m_pPlayer == nullptr)
	{
		return false;
	}

	if (m_moneyCollected < m_extractionGoal)
	{
		char msg[128];
		std::snprintf(
			msg,
			sizeof(msg),
			"Need $%d to extract (collected $%d / $%d).",
			m_extractionGoal - m_moneyCollected,
			m_moneyCollected,
			m_extractionGoal);
		LogManager::getInstance().log(msg);
		return false;
	}

	if (!ProceduralGridLayout::isInsideSpawnExtractionRoom(
			m_pPlayer->x(),
			m_pPlayer->y()))
	{
		LogManager::getInstance().log(
			"Return to the extraction room (where you spawned) to extract.");
		return false;
	}

	char msg[96];
	std::snprintf(
		msg,
		sizeof(msg),
		"Extracted with $%d — mission complete!",
		m_moneyCollected);
	LogManager::getInstance().log(msg);
	Game::GetInstance().Quit();
	return true;
}

void SceneCardBoard::Draw(Renderer& renderer) {
	m_pMap->drawFloor(*m_pRenderer);
	drawExtractionZone();

	const bool navDebug = m_pPlayer != nullptr && m_pPlayer->showHitboxDebug();
	if (navDebug && m_pEnemies != nullptr && m_pRenderer != nullptr)
	{
		m_pEnemies->drawNavDebug(
			*m_pRenderer,
			mCameraX,
			mCameraY,
			static_cast<float>(m_pRenderer->getWidth()),
			static_cast<float>(m_pRenderer->getHeight()));
	}

	if (m_pEnemies != nullptr)
	{
		m_pEnemies->draw(*m_pRenderer);
	}

	if (navDebug && m_pEnemies != nullptr && m_pRenderer != nullptr)
	{
		m_pEnemies->drawPathDebug(*m_pRenderer);
	}

	m_pPlayer->drawFlashlightMask(*m_pRenderer, *m_pMap, mCameraX, mCameraY);

	// Draw walls after the vision mask so every collision segment is visible in the lit view.
	m_pMap->drawWalls(*m_pRenderer);

	m_pPlayer->drawNoisePulses(*m_pRenderer);
	m_pPlayer->drawSprite(*m_pRenderer);
	m_pPlayer->drawHitboxDebug(*m_pRenderer);

	m_pPlayer->drawFlashlightMeter(*m_pRenderer, mCameraX, mCameraY);
	
	m_pUI->draw(*m_pRenderer, mCameraX, mCameraY);

	const float viewW = static_cast<float>(m_pRenderer->getWidth());
	const float viewH = static_cast<float>(m_pRenderer->getHeight());
	m_pUI->drawInventoryHud(
		*m_pRenderer,
		mCameraX,
		mCameraY,
		viewW,
		viewH,
		m_inventory);
	m_pUI->drawExtractionHud(
		*m_pRenderer,
		mCameraX,
		mCameraY,
		viewW,
		viewH,
		m_moneyCollected,
		m_extractionGoal);

	// Minimap after UI so it sits on top; uses camera+offset to stay in screen corner.
	if (m_pMinimap != nullptr && m_pMap != nullptr && m_pPlayer != nullptr && m_pEnemies != nullptr)
	{
		m_pMinimap->draw(
			*m_pRenderer,
			mCameraX,
			mCameraY,
			static_cast<float>(m_pRenderer->getWidth()),
			static_cast<float>(m_pRenderer->getHeight()),
			*m_pMap,
			*m_pPlayer,
			*m_pEnemies);
	}

	for (Item* battery : m_batteries)
	{
		if (battery != nullptr)
		{
			battery->Draw(renderer);
		}
	}

	for (Item* loot : m_worldLoot)
	{
		if (loot != nullptr)
		{
			loot->Draw(renderer);
		}
	}

	//Didnt need this for renderering, adding it lead to flickering.
	//m_pRenderer->present();
}

void SceneCardBoard::DebugDraw() {
	int remaining = 0;
	int stolen = 0;
	for (Item* battery : m_batteries)
	{
		if (battery == nullptr)
		{
			continue;
		}
		if (battery->isCollected())
		{
			continue;
		}
		if (battery->isStolen())
		{
			++stolen;
		}
		else
		{
			++remaining;
		}
	}
	ImGui::Text("Batteries collected: %d", mCollectedItems);
	ImGui::Text("Batteries remaining: %d (stolen: %d)", remaining, stolen);
	ImGui::Text("Loot value collected: $%d / $%d", m_moneyCollected, m_extractionGoal);
	ImGui::Text("Inventory slot selected: %d", m_inventory.selectedSlot() + 1);
	if (m_pPlayer != nullptr)
	{
		ImGui::Text(
			"H — world debug: %s (green=BFS nodes, yellow=paths, cyan=next waypoint)",
			m_pPlayer->showHitboxDebug() ? "ON" : "off");
	}
	if (m_pEnemies != nullptr)
	{
		m_pEnemies->debugDraw();
	}
	if (m_pMinimap != nullptr)
	{
		m_pMinimap->debugDraw();
	}
}

void SceneCardBoard::DrawHudOverlay()
{
	if (m_pRenderer == nullptr)
	{
		return;
	}

	const float viewW = static_cast<float>(m_pRenderer->getWidth());
	const float viewH = static_cast<float>(m_pRenderer->getHeight());

	if (m_pUI != nullptr)
	{
		m_pUI->drawInventoryKeyLabels(
			mCameraX,
			mCameraY,
			viewW,
			viewH,
			m_inventory);
	}

	const int balance = m_moneyCollected - m_extractionGoal;
	const float progress = extractionProgressTowardZero(m_moneyCollected, m_extractionGoal);
	float red = 0.95f;
	float green = 0.12f;
	float blue = 0.12f;
	extractionMoneyColor(progress, red, green, blue);

	ImGui::SetNextWindowPos(ImVec2(viewW - 248.0f, viewH - 108.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(228.0f, 96.0f), ImGuiCond_Always);
	ImGui::Begin(
		"Extract",
		nullptr,
		ImGuiWindowFlags_NoDecoration
			| ImGuiWindowFlags_NoInputs
			| ImGuiWindowFlags_NoBackground
			| ImGuiWindowFlags_NoNav);

	char moneyLine[64];
	std::snprintf(moneyLine, sizeof(moneyLine), "$%d", balance);
	ImGui::TextColored(ImVec4(red, green, blue, 1.0f), "%s", moneyLine);
	ImGui::Text("Goal: $%d  (E pick up, Q drop)", m_extractionGoal);

	const bool fundsReady = m_moneyCollected >= m_extractionGoal;
	const bool inExtractRoom = m_pPlayer != nullptr
		&& ProceduralGridLayout::isInsideSpawnExtractionRoom(
			m_pPlayer->x(),
			m_pPlayer->y());

	if (fundsReady && inExtractRoom)
	{
		ImGui::TextColored(ImVec4(0.2f, 0.95f, 0.35f, 1.0f), "Press F to extract");
	}
	else if (fundsReady)
	{
		ImGui::TextColored(ImVec4(0.95f, 0.85f, 0.25f, 1.0f), "Return to spawn room");
	}
	else
	{
		ImGui::Text("Extract at spawn room when funded");
	}

	ImGui::End();
}

