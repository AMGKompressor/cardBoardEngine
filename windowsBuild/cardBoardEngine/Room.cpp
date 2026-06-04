#include "Room.h"

// -----------------------------------------------------------------------------
// Room layout — local space 640x640, origin top-left, +Y down
//
//  (0,0)────[270..370 gap]────(640,0)    North
//    |                            |
// [gap]                         [gap]    West / East
//    |                            |
//  (0,640)──[270..370 gap]──(640,640)   South
//
// All 4 room types have all 4 sides open.
// Internal walls are axis-aligned and connect to an outer wall on at least one end.
// Room 5 (Closed) has no openings and no internal walls.
// -----------------------------------------------------------------------------

Room::Room(uint8_t type)
    : mType(type)
{
    makeWalls();
}

Room::Room(Room&&) noexcept = default;
Room& Room::operator=(Room&&) noexcept = default;

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
    const float S = kRoomSize;   // 640
    const float d0 = 270.0f;      // doorway gap start
    const float d1 = 370.0f;      // doorway gap end

    // ------------------------------------------------------------------
    // Outer walls — shared by all open room types (1-4)
    // Each side is split into two segments around the doorway gap.
    // Room 5 (Closed) gets one full segment per side instead.
    // ------------------------------------------------------------------
    if (mType == ROOM_CLOSED)
    {
        addHor(0.f, 0.f, S);   // North — full
        addHor(S, 0.f, S);   // South — full
        addVer(0.f, 0.f, S);   // West  — full
        addVer(S, 0.f, S);   // East  — full
        return;
    }

    // North wall (y=0) — gap at x: 270..370
    addHor(0.f, 0.f, d0);
    addHor(0.f, d1, S);

    // South wall (y=640) — gap at x: 270..370
    addHor(S, 0.f, d0);
    addHor(S, d1, S);

    // West wall (x=0) — gap at y: 270..370
    addVer(0.f, 0.f, d0);
    addVer(0.f, d1, S);

    // East wall (x=640) — gap at y: 270..370
    addVer(S, 0.f, d0);
    addVer(S, d1, S);

    // ------------------------------------------------------------------
    // Internal walls — unique per room type
    // ------------------------------------------------------------------
    switch (mType)
    {
    case ROOM_1:
        // One horizontal wall stub from the West outer wall, upper area.
        // Runs from x=0 to x=200, sitting at y=180.
        addHor(180.f, 0.f, 200.f);
        break;

    case ROOM_2:
        // Four wall stubs, one from each outer wall pointing inward —
        // creates a pinwheel/maze forcing the player to navigate around.
        addHor(160.f, 0.f, 220.f);   // from West wall,  upper
        addHor(480.f, 420.f, S);       // from East wall,  lower
        addVer(160.f, 0.f, 220.f);   // from North wall, left
        addVer(480.f, 420.f, S);       // from South wall, right
        break;

    case ROOM_3:
        // Long vertical wall from North outer wall down to y=500,
        // positioned at x=400 (right of centre).
        // Short vertical stub from West outer wall at y=480.
        addVer(400.f, 0.f, 500.f);   // main divider — North wall down
        addVer(0.f, 480.f, S);       // short stub   — West wall down to South
        break;

    case ROOM_4:
        // Two horizontal walls creating a staggered chicane:
        // Upper wall from East outer wall extending left.
        // Lower wall from West outer wall extending right.
        addHor(213.f, 320.f, S);       // upper — East wall leftward
        addHor(426.f, 0.f, 320.f);   // lower — West wall rightward
        break;

    default:
        break;
    }
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
