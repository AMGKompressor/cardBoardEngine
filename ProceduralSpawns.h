#pragma once

// Spawn helpers for the procedural 5x3 room grid (CBE).
// Wall geometry lives in Room.cpp / Map::generate() — not here.

#include "EnemyType.h"
#include "Map_1.h"

#include <vector>

class NavGrid;

namespace ProceduralGridLayout
{
	constexpr float kRoomSize = 640.0f;

	constexpr float roomCenterX(int col)
	{
		return static_cast<float>(col) * kRoomSize + kRoomSize * 0.5f;
	}

	constexpr float roomCenterY(int row)
	{
		return static_cast<float>(row) * kRoomSize + kRoomSize * 0.5f;
	}

	struct RoomBounds
	{
		float left = 0.0f;
		float top = 0.0f;
		float right = 0.0f;
		float bottom = 0.0f;
	};

	inline RoomBounds spawnExtractionRoomBounds()
	{
		const float left =
			static_cast<float>(Map::kSpawnRoomCol) * kRoomSize;
		const float top =
			static_cast<float>(Map::kSpawnRoomRow) * kRoomSize;
		return { left, top, left + kRoomSize, top + kRoomSize };
	}

	inline float spawnExtractionRoomCenterX()
	{
		return roomCenterX(Map::kSpawnRoomCol);
	}

	inline float spawnExtractionRoomCenterY()
	{
		return roomCenterY(Map::kSpawnRoomRow);
	}

	inline bool isInsideSpawnExtractionRoom(float worldX, float worldY, float inset = 56.0f)
	{
		const RoomBounds room = spawnExtractionRoomBounds();
		return worldX >= room.left + inset
			&& worldX <= room.right - inset
			&& worldY >= room.top + inset
			&& worldY <= room.bottom - inset;
	}

	// Random walkable positions each run (after NavGrid is built).
	std::vector<EnemySpawnSpec> generateRandomEnemySpawns(
		const NavGrid& nav,
		float playerX,
		float playerY);

	namespace BatterySpawns
	{
		struct Point
		{
			float x = 0.0f;
			float y = 0.0f;
		};

		inline constexpr Point kList[] = {
			{ roomCenterX(0), roomCenterY(0) },
			{ roomCenterX(4), roomCenterY(0) },
			{ roomCenterX(2), roomCenterY(2) },
		};

		constexpr int kCount = static_cast<int>(sizeof(kList) / sizeof(kList[0]));
	}

	namespace LootSpawns
	{
		struct Point
		{
			float x = 0.0f;
			float y = 0.0f;
		};

		inline constexpr Point kList[] = {
			{ roomCenterX(0), roomCenterY(2) },
			{ roomCenterX(1), roomCenterY(0) },
			{ roomCenterX(2), roomCenterY(0) },
			{ roomCenterX(3), roomCenterY(2) },
			{ roomCenterX(4), roomCenterY(2) },
			{ roomCenterX(0), roomCenterY(1) },
			{ roomCenterX(4), roomCenterY(0) },
			{ roomCenterX(3), roomCenterY(1) },
		};

		constexpr int kCount = static_cast<int>(sizeof(kList) / sizeof(kList[0]));
	}
}
