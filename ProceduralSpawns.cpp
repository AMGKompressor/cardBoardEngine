#include "ProceduralSpawns.h"

#include "NavGrid.h"

#include <cmath>
#include <cstdlib>
#include <vector>

namespace
{
	constexpr float kMinDistFromPlayer = 420.0f;
	constexpr float kMinDistBetweenEnemies = 280.0f;
	constexpr int kMaxPickAttempts = 160;

	bool isFarEnoughFromPoints(
		float x,
		float y,
		float avoidX,
		float avoidY,
		float avoidMinDist,
		const std::vector<EnemySpawnSpec>& placed,
		float placedMinDist)
	{
		const float avoidDx = x - avoidX;
		const float avoidDy = y - avoidY;
		const float avoidMinSq = avoidMinDist * avoidMinDist;
		if (avoidDx * avoidDx + avoidDy * avoidDy < avoidMinSq)
		{
			return false;
		}

		const float placedMinSq = placedMinDist * placedMinDist;
		for (const EnemySpawnSpec& spawn : placed)
		{
			const float dx = x - spawn.worldX;
			const float dy = y - spawn.worldY;
			if (dx * dx + dy * dy < placedMinSq)
			{
				return false;
			}
		}

		return true;
	}

	bool pickRandomWalkableSpawn(
		const NavGrid& nav,
		float playerX,
		float playerY,
		const std::vector<EnemySpawnSpec>& placed,
		float& outX,
		float& outY)
	{
		if (!nav.isBuilt())
		{
			return false;
		}

		for (int attempt = 0; attempt < kMaxPickAttempts; ++attempt)
		{
			const int gx = std::rand() % nav.gridWidth();
			const int gy = std::rand() % nav.gridHeight();

			float wx = 0.0f;
			float wy = 0.0f;
			nav.cellToWorld(gx, gy, wx, wy);
			if (!nav.isWalkableWorld(wx, wy))
			{
				continue;
			}

			if (!isFarEnoughFromPoints(
					wx,
					wy,
					playerX,
					playerY,
					kMinDistFromPlayer,
					placed,
					kMinDistBetweenEnemies))
			{
				continue;
			}

			outX = wx;
			outY = wy;
			return true;
		}

		return false;
	}

	bool pickRandomRoomCenterSpawn(
		float playerX,
		float playerY,
		const std::vector<EnemySpawnSpec>& placed,
		float& outX,
		float& outY)
	{
		for (int attempt = 0; attempt < kMaxPickAttempts; ++attempt)
		{
			const int col = std::rand() % Map::kGridCols;
			const int row = std::rand() % Map::kGridRows;

			const float wx = ProceduralGridLayout::roomCenterX(col);
			const float wy = ProceduralGridLayout::roomCenterY(row);

			if (!isFarEnoughFromPoints(
					wx,
					wy,
					playerX,
					playerY,
					kMinDistFromPlayer,
					placed,
					kMinDistBetweenEnemies))
			{
				continue;
			}

			outX = wx;
			outY = wy;
			return true;
		}

		return false;
	}
}

namespace ProceduralGridLayout
{
	std::vector<EnemySpawnSpec> generateRandomEnemySpawns(
		const NavGrid& nav,
		float playerX,
		float playerY)
	{
		static const EnemyType kTypes[] = {
			EnemyType::Spirit,
			EnemyType::Spirit,
			EnemyType::Hidden,
			EnemyType::Hidden,
			EnemyType::Revenant,
			EnemyType::Revenant,
		};
		constexpr int kEnemyCount = static_cast<int>(sizeof(kTypes) / sizeof(kTypes[0]));

		std::vector<EnemySpawnSpec> spawns;
		spawns.reserve(static_cast<std::size_t>(kEnemyCount));

		for (int i = 0; i < kEnemyCount; ++i)
		{
			float spawnX = 0.0f;
			float spawnY = 0.0f;
			const bool picked = pickRandomWalkableSpawn(nav, playerX, playerY, spawns, spawnX, spawnY)
				|| pickRandomRoomCenterSpawn(playerX, playerY, spawns, spawnX, spawnY);

			if (!picked)
			{
				break;
			}

			spawns.push_back({ kTypes[i], spawnX, spawnY });
		}

		return spawns;
	}
}
