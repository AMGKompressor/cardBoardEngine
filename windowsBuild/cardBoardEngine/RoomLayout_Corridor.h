// RoomLayout_Corridor.h
#pragma once

#include "BasicMapLayout_1.h" // WallSegment, WallSpec, already defined here
#include <array>

namespace RoomLayout_Corridor
{
    constexpr WallSpec Hor(float y, float xa, float xb)
    {
        return { xa, y, xb, y };
    }
    constexpr WallSpec Ver(float x, float ya, float yb)
    {
        return { x, ya, x, yb };
    }

    namespace detail
    {
        template <std::size_t N>
        constexpr std::array<WallSegment, N> buildWalls(const BasicMapLayout::WallSpec(&src)[N])
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

    inline constexpr BasicMapLayout::WallSpec kWallData[] =
    {
        Hor(0.f,   0.f, 640.f),   // North wall
        Hor(640.f, 0.f, 640.f),   // South wall
        Ver(0.f,   0.f, 270.f),   // West wall top
        Ver(0.f,   370.f, 640.f), // West wall bottom
        Ver(640.f, 0.f,   270.f), // East wall top
        Ver(640.f, 370.f, 640.f), // East wall bottom
    };

    inline constexpr auto kWalls = detail::buildWalls(kWallData);
}