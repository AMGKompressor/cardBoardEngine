// Map_1.h ù procedural 5x3 room grid (CBE)
#pragma once

#include "Room.h"

#include <vector>

class Renderer;

class Map
{
public:
	Map();
	~Map();

	static constexpr int kGridCols = 5;
	static constexpr int kGridRows = 3;

	// Player spawn and extraction zone (center cell of the 5x3 grid).
	static constexpr int kSpawnRoomCol = 2;
	static constexpr int kSpawnRoomRow = 1;

	void generate();
	int mLayout[kGridRows][kGridCols] = {};

	static constexpr float kWidth = Room::kRoomSize * kGridCols;
	static constexpr float kHeight = Room::kRoomSize * kGridRows;

	float width() const { return mWidth; }
	float height() const { return mHeight; }
	const float* wireFlat() const { return mWireFlat.data(); }
	int segmentCount() const { return mSegmentCount; }

	void constrainPlayerCenter(
		float& cx,
		float& cy,
		float worldHalfX,
		float worldHalfY,
		float hitboxHalfW,
		float hitboxHalfH,
		float facingDeg) const;

	bool pollNewWallContact(
		float cx,
		float cy,
		float hitboxHalfW,
		float hitboxHalfH,
		float facingDeg,
		float touchSlop = 3.0f) const;

	void drawWalls(Renderer& renderer) const;
	void drawFloor(Renderer& renderer) const;

private:
	void setNewMap(int(&layout)[kGridRows][kGridCols]);
	void bake();

	float mWidth = kWidth;
	float mHeight = kHeight;
	std::vector<float> mWireFlat;
	int mSegmentCount = 0;
	mutable std::vector<bool> mPrevWallTouch;
	std::vector<Room> mRooms;
};
