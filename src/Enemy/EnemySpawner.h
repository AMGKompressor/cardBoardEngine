// cardBoard -- owns and ticks all active enemies; handles spawn timing
#pragma once

#include "Enemy.h"
#include "EnemyConfig.h"

#include <vector>

class Renderer;

namespace CardBoard
{
	class Map;
	class Player;

	struct SpawnPoint
	{
		float x = 0.0f;
		float y = 0.0f;
	};

	class EnemySpawner
	{
	public:
		bool initialize(Renderer& renderer, const Map& map);
		void shutdown();

		void update(float deltaTime, const Map& map, Player& player);

		void drawSprites(Renderer& renderer) const;
		void drawDebug(Renderer& renderer) const;

		void setShowDebug(bool show);
		bool showDebug() const { return mShowDebug; }

	private:
		void spawnAt(Renderer& renderer, float x, float y);
		void purgeDeadEnemies();

		Renderer* mRenderer = nullptr;
		EnemyConfig         mEnemyConfig;
		std::vector<Enemy*> mEnemies;

		float mSpawnInterval = 9998.0f;
		float mSpawnTimer = 0.0f;
		int   mSpawnIndex = 0;

		bool  mShowDebug = false;

		static constexpr SpawnPoint kSpawnPoints[] = {
			{  200.0f,  780.0f },
			{ 2400.0f,  250.0f },
			{ 1400.0f, 1450.0f },
			{ 2300.0f, 1000.0f },
		};
		static constexpr int kSpawnPointCount =
			static_cast<int>(sizeof(kSpawnPoints) / sizeof(kSpawnPoints[0]));
	};
}