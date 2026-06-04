// cardBoard -- EnemySpawner implementation
#include "EnemySpawner.h"

#include "Map/Map.h"
#include "Player/Player.h"

#include "renderer.h"

#include <algorithm>

namespace CardBoard
{
	bool EnemySpawner::initialize(Renderer& renderer, const Map& /*map*/)
	{
		shutdown();
		mRenderer = &renderer;
		mSpawnTimer = mSpawnInterval * 0.5f;

		for (int i = 0; i < kSpawnPointCount; ++i)
		{
			spawnAt(renderer, kSpawnPoints[i].x, kSpawnPoints[i].y);
		}

		return true;
	}

	void EnemySpawner::shutdown()
	{
		for (Enemy* e : mEnemies)
		{
			e->shutdown();
			delete e;
		}
		mEnemies.clear();
		mRenderer = nullptr;
		mSpawnTimer = 0.0f;
		mSpawnIndex = 0;
	}

	void EnemySpawner::update(float deltaTime, const Map& map, Player& player)
	{
		for (Enemy* e : mEnemies)
		{
			e->update(deltaTime, map, player);
		}

		purgeDeadEnemies();

		mSpawnTimer -= deltaTime;
		if (mSpawnTimer <= 0.0f && mRenderer != nullptr)
		{
			mSpawnTimer = mSpawnInterval;
			const SpawnPoint& sp = kSpawnPoints[mSpawnIndex % kSpawnPointCount];
			spawnAt(*mRenderer, sp.x, sp.y);
			++mSpawnIndex;
		}
	}

	void EnemySpawner::drawSprites(Renderer& renderer) const
	{
		for (const Enemy* e : mEnemies)
		{
			e->drawSprite(renderer);
		}
	}

	void EnemySpawner::drawDebug(Renderer& renderer) const
	{
		if (!mShowDebug)
		{
			return;
		}
		for (const Enemy* e : mEnemies)
		{
			e->drawDebug(renderer);
		}
	}

	void EnemySpawner::setShowDebug(bool show)
	{
		mShowDebug = show;
		for (Enemy* e : mEnemies)
		{
			e->setShowDebug(show);
		}
	}

	void EnemySpawner::spawnAt(Renderer& renderer, float x, float y)
	{
		Enemy* e = new Enemy();
		if (e->initialize(renderer, mEnemyConfig, x, y))
		{
			e->setShowDebug(mShowDebug);
			mEnemies.push_back(e);
		}
		else
		{
			delete e;
		}
	}

	void EnemySpawner::purgeDeadEnemies()
	{
		for (Enemy* e : mEnemies)
		{
			if (e->isDead())
			{
				e->shutdown();
				delete e;
			}
		}
		mEnemies.erase(
			std::remove_if(
				mEnemies.begin(),
				mEnemies.end(),
				[](const Enemy* e) { return e->isDead(); }),
			mEnemies.end());
	}
}