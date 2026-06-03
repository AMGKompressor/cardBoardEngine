// cardBoard ù default tutorial-style wall layout (+Y down like screen)
//
// LEVEL DESIGN CHEAT SHEET (edit kWallData[], then rebuild)
// ------------------------------------------------------------
// ù Hor(y, xStart, xEnd)  ù horizontal wall at pixel row y
// ù Ver(x, yStart, yEnd)  ù vertical wall at pixel column x
// ù { x0, y0, x1, y1 }    ù diagonal or any angled wall
// ù Leave a GAP between segment endpoints to make a doorway (no line = no block)
// ù Segments must not cross the playable area you want open unless intentional
// ù Loaded by Map::loadBasicTutorial() ù see Map_1.cpp for pipeline
//
#pragma once

#include "EnemyType.h"

#include <array>
#include <cstddef>

struct WallSegment
{
	float x0 = 0.0f;
	float y0 = 0.0f;
	float x1 = 0.0f;
	float y1 = 0.0f;
};

struct WallSpec
{
	float x0 = 0.0f;
	float y0 = 0.0f;
	float x1 = 0.0f;
	float y1 = 0.0f;
	float shiftX = 0.0f;
	float shiftY = 0.0f;
};

namespace BasicMapLayout
{
	constexpr float kEntryWest = 80.0f;
	constexpr float kEntryEast = kEntryWest + (1000.0f - kEntryWest) * 1.35f;

	constexpr WallSpec Hor(float y, float xa, float xb) { return { xa, y, xb, y }; }
	constexpr WallSpec Ver(float x, float ya, float yb) { return { x, ya, x, yb }; }

	namespace detail
	{
		template <std::size_t N>
		constexpr std::array<WallSegment, N> buildWalls(const WallSpec(&src)[N])
		{
			std::array<WallSegment, N> out{};
			for (std::size_t i = 0; i < N; ++i)
			{
				out[i].x0 = src[i].x0 + src[i].shiftX;
				out[i].y0 = src[i].y0 + src[i].shiftY;
				out[i].x1 = src[i].x1 + src[i].shiftX;
				out[i].y1 = src[i].y1 + src[i].shiftY;
			}
			return out;
		}
	}

	inline constexpr WallSpec kWallData[] = {
		Hor(1200.0f, kEntryWest, kEntryEast),
		Ver(kEntryEast, 1200.0f, 1500.0f),
		Hor(1500.0f, 1320.0f, 2480.0f),
		Ver(2480.0f, 1500.0f, 190.0f),
		Hor(190.0f, 2480.0f, 1750.0f),
		{ 1750.0f, 190.0f, kEntryEast, 720.0f },
		Hor(720.0f, kEntryEast, kEntryWest),
		Ver(kEntryWest, 720.0f, 880.0f),
		Ver(kEntryWest, 1040.0f, 1200.0f),
		Hor(960.0f, kEntryEast, 2480.0f),
	};

	inline constexpr auto kWalls = detail::buildWalls(kWallData);
	constexpr int kWallCount = static_cast<int>(kWalls.size());

	// Enemy spawn points (world x,y). Edit kList[] per level.
	namespace EnemySpawns
	{
		inline constexpr EnemySpawnSpec kList[] = {
			{ EnemyType::Spirit, 320.0f, 780.0f },    // West corridor
			{ EnemyType::Spirit, 2100.0f, 450.0f },   // North-east gallery
			{ EnemyType::Hidden, 1900.0f, 820.0f },   // East hallway
			{ EnemyType::Hidden, 1150.0f, 1080.0f },  // South hall
			{ EnemyType::Revenant, 2350.0f, 650.0f }, // Far east
			{ EnemyType::Revenant, 1550.0f, 480.0f }, // North central
		};

		constexpr int kCount = static_cast<int>(sizeof(kList) / sizeof(kList[0]));
	}

	// Three battery pickups (world x,y).
	namespace BatterySpawns
	{
		struct Point
		{
			float x = 0.0f;
			float y = 0.0f;
		};

		inline constexpr Point kList[] = {
			{ 850.0f, 850.0f },   // Main hall (east of entry)
			{ 1050.0f, 1020.0f }, // South corridor
			{ 1650.0f, 780.0f },  // East wing
		};

		constexpr int kCount = static_cast<int>(sizeof(kList) / sizeof(kList[0]));
	}
}
