#include "Room.h"

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
	const float S = kRoomSize;
	const float d0 = 270.0f;
	const float d1 = 370.0f;

	if (mType == ROOM_CLOSED)
	{
		addHor(0.f, 0.f, S);
		addHor(S, 0.f, S);
		addVer(0.f, 0.f, S);
		addVer(S, 0.f, S);
		return;
	}

	addHor(0.f, 0.f, d0);
	addHor(0.f, d1, S);

	addHor(S, 0.f, d0);
	addHor(S, d1, S);

	addVer(0.f, 0.f, d0);
	addVer(0.f, d1, S);

	addVer(S, 0.f, d0);
	addVer(S, d1, S);

	switch (mType)
	{
	case ROOM_0:
		break;
	case ROOM_1:
		addHor(180.f, 0.f, 200.f);
		break;
	case ROOM_2:
		addHor(160.f, 0.f, 220.f);
		addHor(480.f, 420.f, S);
		addVer(160.f, 0.f, 220.f);
		addVer(480.f, 420.f, S);
		break;
	case ROOM_3:
		addVer(400.f, 0.f, 500.f);
		addVer(0.f, 480.f, S);
		break;
	case ROOM_4:
		addHor(213.f, 320.f, S);
		addHor(426.f, 0.f, 320.f);
		break;
	default:
		break;
	}
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
