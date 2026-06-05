#include "EnemyManager.h"

#include "ProceduralSpawns.h"
#include "Enemy.h"
#include "Player_1.h"
#include "PlayerConfig_1.h"
#include "logmanager.h"
#include "renderer.h"

#include "imgui.h"

#include <cstdio>

EnemyManager::EnemyManager() = default;

EnemyManager::~EnemyManager()
{
	shutdown();
}

void EnemyManager::syncHearingFromPlayerConfig(const PlayerConfig& playerConfig)
{
	mConfig.hearRadiusWalk = playerConfig.footstep.walkNoiseRadius;
	mConfig.hearRadiusSprint = playerConfig.footstep.sprintNoiseRadius;
	mConfig.playerAwarenessRadius = playerConfig.footstep.sprintNoiseRadius;
	mConfig.hiddenStealRadius = playerConfig.footstep.walkNoiseRadius;
}

bool EnemyManager::initialize(Renderer& renderer, float playerSpawnX, float playerSpawnY)
{
	shutdown();

	mSpawns = ProceduralGridLayout::generateRandomEnemySpawns(
		mNavGrid,
		playerSpawnX,
		playerSpawnY);

	mEnemyCount = static_cast<int>(mSpawns.size());
	if (mEnemyCount < 1)
	{
		LogManager::getInstance().log("EnemyManager: no random spawn points found.");
		return false;
	}
	mEnemies = new Enemy[mEnemyCount];

	for (int i = 0; i < mEnemyCount; ++i)
	{
		const EnemySpawnSpec& spec = mSpawns[static_cast<std::size_t>(i)];

		if (!mEnemies[i].initialize(spec.type, renderer, mConfig, spec.worldX, spec.worldY))
		{
			LogManager::getInstance().log("EnemyManager: failed to spawn enemy.");
			return false;
		}
	}

	char buf[96];
	std::snprintf(
		buf,
		sizeof(buf),
		"%d enemies placed at random walkable map positions.",
		mEnemyCount);
	LogManager::getInstance().log(buf);
	return true;
}

bool EnemyManager::buildNavigation(const Map& map)
{
	const bool ok = mNavGrid.buildFromMap(map, 48.0f, 36.0f);
	if (ok)
	{
		LogManager::getInstance().log(
			"NavGrid BFS ready for enemy pathfinding (48px cells).");
	}
	else
	{
		LogManager::getInstance().log("NavGrid build failed.");
	}
	return ok;
}

const Enemy* EnemyManager::enemyAt(int index) const
{
	if (mEnemies == nullptr || index < 0 || index >= mEnemyCount)
	{
		return nullptr;
	}
	return &mEnemies[index];
}

void EnemyManager::shutdown()
{
	delete[] mEnemies;
	mEnemies = nullptr;
	mEnemyCount = 0;
	mSpawns.clear();
}

void EnemyManager::update(
	float deltaTime,
	const Map& map,
	Player& player,
	const std::vector<Item*>& worldItems)
{
	if (mEnemies == nullptr)
	{
		return;
	}

	player.resetEnemySanityDrain();

	const NavGrid* nav = mNavGrid.isBuilt() ? &mNavGrid : nullptr;
	for (int i = 0; i < mEnemyCount; ++i)
	{
		mEnemies[i].update(deltaTime, map, player, worldItems, mConfig, nav);
	}
}

void EnemyManager::draw(Renderer& renderer) const
{
	if (mEnemies == nullptr)
	{
		return;
	}

	for (int i = 0; i < mEnemyCount; ++i)
	{
		mEnemies[i].draw(renderer);
	}
}

void EnemyManager::drawNavDebug(
	Renderer& renderer,
	float cameraX,
	float cameraY,
	float viewWidth,
	float viewHeight) const
{
	if (mNavGrid.isBuilt())
	{
		mNavGrid.drawDebug(renderer, cameraX, cameraY, viewWidth, viewHeight);
	}
}

void EnemyManager::drawPathDebug(Renderer& renderer) const
{
	if (mEnemies == nullptr)
	{
		return;
	}

	for (int i = 0; i < mEnemyCount; ++i)
	{
		mEnemies[i].drawPathDebug(renderer);
	}
}

void EnemyManager::debugDraw() const
{
	if (mNavGrid.isBuilt())
	{
		ImGui::Text(
			"Nav grid: %d x %d cells (%.0fpx), walkable nodes in view when H is on",
			mNavGrid.gridWidth(),
			mNavGrid.gridHeight(),
			mNavGrid.cellSize());
	}
	else
	{
		ImGui::Text("Nav grid: not built");
	}

	if (mEnemies == nullptr)
	{
		return;
	}

	for (int i = 0; i < mEnemyCount; ++i)
	{
		mEnemies[i].debugDraw();
	}
}
