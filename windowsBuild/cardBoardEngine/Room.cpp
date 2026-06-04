// Room.cpp
#include "Room.h"

Room::Room(uint8_t doorways)
    : mDoorways(doorways)
{
    makeWalls();
}

void Room::addHor(float y, float xa, float xb)
{
    mWalls.push_back({ xa, y, xb, y });
}

void Room::addVer(float x, float ya, float yb)
{
    mWalls.push_back({ x, ya, x, yb });
}

void Room::makeWalls()
{
    const float S = kRoomSize;
    const float d0 = S * 0.5f - kDoorWidth * 0.5f;   // 270 — gap start
    const float d1 = S * 0.5f + kDoorWidth * 0.5f;   // 370 — gap end

    // North wall  (y = 0)
    if (mDoorways & DOOR_NORTH) { addHor(0.f, 0.f, d0);  addHor(0.f, d1, S); }
    else { addHor(0.f, 0.f, S); }

    // South wall  (y = S)
    if (mDoorways & DOOR_SOUTH) { addHor(S, 0.f, d0);  addHor(S, d1, S); }
    else { addHor(S, 0.f, S); }

    // West wall   (x = 0)
    if (mDoorways & DOOR_WEST) { addVer(0.f, 0.f, d0);  addVer(0.f, d1, S); }
    else { addVer(0.f, 0.f, S); }

    // East wall   (x = S)
    if (mDoorways & DOOR_EAST) { addVer(S, 0.f, d0);  addVer(S, d1, S); }
    else { addVer(S, 0.f, S); }
}

void Room::appendWallsToBuffer(std::vector<float>& outBuffer,
    float offsetX, float offsetY) const
{
    for (const WallSegment& w : mWalls)
    {
        outBuffer.push_back(w.x0 + offsetX);
        outBuffer.push_back(w.y0 + offsetY);
        outBuffer.push_back(w.x1 + offsetX);
        outBuffer.push_back(w.y1 + offsetY);
    }
}