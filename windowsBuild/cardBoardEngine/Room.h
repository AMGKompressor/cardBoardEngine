#pragma once
#include "WallSegment.h"
#include <vector>
#include <cstdint>

class Renderer;

// -----------------------------------------------------------------------------
// Room
// A fixed 640x640 cell. All rooms 1-4 are open on all 4 sides with the same
// doorway gap (270..370). Each has unique internal walls. Room 5 is fully closed.
// -----------------------------------------------------------------------------
class Room
{
public:
    // Room type constants — used as index in Map::kLayout matrix
    enum RoomType : uint8_t
    {
        ROOM_0 = 1, // OPEN ROOM
        ROOM_1 = 2,   // horizontal stub from West wall
        ROOM_2 = 3,   // four inward stubs — pinwheel layout
        ROOM_3 = 4,   // long vertical divider + West stub
        ROOM_4 = 5,   // staggered horizontal chicane
        ROOM_CLOSED = 6,   // fully sealed, no openings, no internal walls
    };

    static constexpr float kRoomSize = 640.0f;
    static constexpr float kDoorWidth = 100.0f;

    explicit Room(uint8_t type);

    Room(Room&&) noexcept;
    Room& operator=(Room&&) noexcept;

    // Writes world-space wall data into outBuffer as [x0,y0,x1,y1, ...]
    void appendWallsToBuffer(std::vector<float>& outBuffer,
        float offsetX, float offsetY) const;

    int     wallCount() const { return static_cast<int>(mWalls.size()); }
    uint8_t type()      const { return mType; }

private:
    Room(const Room&) = delete;
    Room& operator=(const Room&) = delete;

    void makeWalls();
    void addHor(float y, float xa, float xb);
    void addVer(float x, float ya, float yb);

    uint8_t                  mType;
    std::vector<WallSegment> mWalls;
};
