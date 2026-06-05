// cardBoard — default tutorial-style wall layout (+Y down like screen)
#pragma once

#include <array>
#include <cstddef>

namespace CardBoard
{
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
			constexpr std::array<WallSegment, N> buildWalls(const WallSpec (&src)[N])
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
	}
}
