// Map_1.h
#pragma once
#include "Room.h"
#include <vector>

class Renderer;

class Map
{
public:
    Map();
    ~Map();

    static constexpr int   kGridCols = 4;
    static constexpr int   kGridRows = 4;
    static constexpr float kDefaultWidth = Room::kRoomSize * kGridCols;   // 2560
    static constexpr float kDefaultHeight = Room::kRoomSize * kGridRows;   // 2560

    void generate();

    float        width()        const { return mWidth; }
    float        height()       const { return mHeight; }
    const float* wireFlat()     const { return mWireFlat.data(); }
    int          segmentCount() const { return mSegmentCount; }

    void constrainPlayerCenter(float& cx, float& cy,
        float worldHalfX, float worldHalfY,
        float hitboxHalfW, float hitboxHalfH,
        float facingDeg) const;

    bool pollNewWallContact(float cx, float cy,
        float hitboxHalfW, float hitboxHalfH,
        float facingDeg,
        float touchSlop = 3.0f) const;

    void drawWalls(Renderer& renderer) const;
    void drawFloor(Renderer& renderer) const;

private:
    void bake();

    float              mWidth = kDefaultWidth;
    float              mHeight = kDefaultHeight;
    std::vector<float> mWireFlat;
    int                mSegmentCount = 0;
    mutable std::vector<bool> mPrevWallTouch;
    std::vector<Room>  mRooms;
};