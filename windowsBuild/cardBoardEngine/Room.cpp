#include "Room.h"
#include "Map_1.h"

#include "BasicMapLayout_1.h"
#include "Collision2D_1.h"

#include "renderer.h"

#include <algorithm>
#include <cmath>
#include <array>

Room::Room(uint8_t doorways) : mDoorways(doorways)
{

}

Room::~Room()
{

}

bool Room::Initialise(Renderer& renderer)
{
    return true;
}

void Room::Process(float deltaTime)
{

}

void Room::Draw(Renderer& renderer)
{

}

template <std::size_t N>
void Room::setWallsFromLayout(const std::array<WallSegment, N>& walls)
{
	mWalls.assign(walls.begin(), walls.end());
}

void Room::appendWallsToBuffer(
    std::vector<float>& outBuffer,
    float offsetX,
    float offsetY) const
{
    for (const WallSegment& w : mWalls)
    {
        outBuffer.push_back(w.x0 + offsetX);
        outBuffer.push_back(w.y0 + offsetY);
        outBuffer.push_back(w.x1 + offsetX);
        outBuffer.push_back(w.y1 + offsetY);
    }
}