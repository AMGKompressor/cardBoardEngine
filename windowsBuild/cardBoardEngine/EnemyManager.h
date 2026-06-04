#pragma once

#include "EnemyType.h"
#include "NavGrid.h"

#include <vector>

class Item;
class Enemy;

class Renderer;
class Map;
class Player;
class Item;
struct PlayerConfig;

namespace EnemySpawns
{
	inline constexpr EnemySpawnSpec kList[] = {
		{ EnemyType::Spirit,   320.0f,  960.0f  },  // room [1][0] centre
		{ EnemyType::Spirit,   960.0f,  320.0f  },  // room [0][1] centre
		{ EnemyType::Hidden,   1600.0f, 960.0f  },  // room [1][2] centre
		{ EnemyType::Hidden,   960.0f,  1600.0f },  // room [2][1] centre
		{ EnemyType::Revenant, 2240.0f, 320.0f  },  // room [0][3] centre
		{ EnemyType::Revenant, 2880.0f, 960.0f  },  // room [1][4] centre
	};

	constexpr int kCount = static_cast<int>(sizeof(kList) / sizeof(kList[0]));
}
class EnemyManager
{
public:
	EnemyManager();
	~EnemyManager();

	bool initialize(Renderer& renderer);
	void syncHearingFromPlayerConfig(const struct PlayerConfig& playerConfig);
	bool buildNavigation(const Map& map);
	void shutdown();

	const NavGrid& navGrid() const { return mNavGrid; }

	// worldItems: batteries Hidden can steal from
	void update(
		float deltaTime,
		const Map& map,
		Player& player,
		const std::vector<Item*>& worldItems);
	void draw(Renderer& renderer) const;
	void drawNavDebug(
		Renderer& renderer,
		float cameraX,
		float cameraY,
		float viewWidth,
		float viewHeight) const;
	void drawPathDebug(Renderer& renderer) const;
	void debugDraw() const;

	const EnemyConfig& config() const { return mConfig; }

	// Used by Minimap to read enemy world positions.
	int enemyCount() const { return mEnemyCount; }
	const Enemy* enemyAt(int index) const;

private:
	EnemyConfig mConfig;
	std::vector<EnemySpawnSpec> mSpawns;
	class Enemy* mEnemies = nullptr;
	int mEnemyCount = 0;
	NavGrid mNavGrid;
};
