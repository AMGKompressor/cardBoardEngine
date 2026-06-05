#pragma once

#include "WallSegment.h"

#include <cstdint>
#include <vector>

class Renderer;

// -----------------------------------------------------------------------------
// Room — one 640x640 cell in the procedural 5x3 grid (from CBE).
// Types 1-4 have doorway gaps on all sides; type 6 is fully sealed.
// -----------------------------------------------------------------------------
class Room
{
public:
	enum RoomType : uint8_t
	{
		ROOM_0 = 1,
		ROOM_1 = 2,
		ROOM_2 = 3,
		ROOM_3 = 4,
		ROOM_4 = 5,
		ROOM_CLOSED = 6,
	};

	static constexpr float kRoomSize = 640.0f;
	static constexpr float kDoorWidth = 100.0f;

	explicit Room(uint8_t type);

	Room(Room&&) noexcept;
	Room& operator=(Room&&) noexcept;

	void appendWallsToBuffer(
		std::vector<float>& outBuffer,
		float offsetX,
		float offsetY) const;

	int wallCount() const { return static_cast<int>(mWalls.size()); }
	uint8_t type() const { return mType; }

private:
	Room(const Room&) = delete;
	Room& operator=(const Room&) = delete;

	void makeWalls();
	void addHor(float y, float xa, float xb);
	void addVer(float x, float ya, float yb);

	uint8_t mType = 0;
	std::vector<WallSegment> mWalls;
};
