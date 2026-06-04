#pragma once

#include "WallSegment.h"
#include <vector>
#include <cstdlib>

class Renderer;

class Room
{
public:
	enum DoorWayFlags : uint8_t
	{
		DOOR_NONE = 0,
		DOOR_NORTH = 1 << 0,
		DOOR_EAST = 1 << 1,
		DOOR_SOUTH = 1 << 2,
		DOOR_WEST = 1 << 3,
	};

	static constexpr float kRoomSize = 640.0f;
	static constexpr float kDoorWidth = 100.0f;

	explicit Room(uint8_t doorways);

	void appendWallsToBuffer(std::vector<float>& outBuffer,
		float offsetX, float offsetY) const;

	int     wallCount()  const { return static_cast<int>(mWalls.size()); }
	uint8_t getDoorways() const { return mDoorways; }

	Room(Room&&) noexcept = default;
	Room& operator=(Room&&) noexcept = default;

private:
	Room(const Room&) = delete;
	Room& operator=(const Room&) = delete;

	void makeWalls();
	void addHor(float y, float xa, float xb);
	void addVer(float x, float ya, float yb);

	uint8_t                  mDoorways;
	std::vector<WallSegment> mWalls;
};

