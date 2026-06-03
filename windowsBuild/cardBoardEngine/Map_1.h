// cardBoard — world bounds, wall segments, collision, and wall drawing
#pragma once

#include "BasicMapLayout_1.h"
#include "Room.h"

#include <array>
#include <vector>

class Renderer;

class Map
{
public:

	Map();
	~Map();

	// Map Size 4x4 = 16 rooms
	static constexpr int kGridCols = 4;
	static constexpr int kGridRows = 4;

	static constexpr float kDefaultWidth = Room::kRoomSize * kGridCols;
	static constexpr float kDefaultHeight = Room::kRoomSize * kGridRows;

	void generate();

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
	void bake();

	template <std::size_t N>
	void setWallsFromLayout(const std::array<WallSegment, N>& walls);

	float mWidth = kDefaultWidth;
	float mHeight = kDefaultHeight;
	std::vector<float> mWireFlat;
	int mSegmentCount = 0;

	mutable std::vector<bool> mPrevWallTouch;

	std::vector<Room> mRooms;
};
