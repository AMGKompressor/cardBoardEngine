// Map_1.cpp — procedural 5x3 room grid (CBE)

#include "Map_1.h"

#include "Collision2D_1.h"
#include "logmanager.h"

#include "renderer.h"

#include <cstdio>

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>

Map::Map()
{
	std::srand(static_cast<unsigned>(std::time(nullptr)));
}

Map::~Map()
{
}

void Map::setNewMap(int(&layout)[kGridRows][kGridCols])
{
	for (int i = 0; i < kGridRows; ++i)
	{
		for (int j = 0; j < kGridCols; ++j)
		{
			layout[i][j] = (std::rand() % 6) + 1;
		}
	}

	// Spawn / extraction room is always open (ROOM_0).
	layout[kSpawnRoomRow][kSpawnRoomCol] = static_cast<int>(Room::ROOM_0);
}

void Map::generate()
{
	int defaultLayout[kGridRows][kGridCols] = {};
	setNewMap(defaultLayout);
	std::memcpy(mLayout, defaultLayout, sizeof(mLayout));

	mRooms.clear();
	mRooms.reserve(kGridRows * kGridCols);

	for (int row = 0; row < kGridRows; ++row)
	{
		for (int col = 0; col < kGridCols; ++col)
		{
			mRooms.emplace_back(static_cast<uint8_t>(mLayout[row][col]));
		}
	}

	mWidth = kWidth;
	mHeight = kHeight;

	bake();
}

void Map::bake()
{
	mWireFlat.clear();

	for (int row = 0; row < kGridRows; ++row)
	{
		for (int col = 0; col < kGridCols; ++col)
		{
			const float offsetX = static_cast<float>(col) * Room::kRoomSize;
			const float offsetY = static_cast<float>(row) * Room::kRoomSize;
			mRooms[static_cast<std::size_t>(row * kGridCols + col)]
				.appendWallsToBuffer(mWireFlat, offsetX, offsetY);
		}
	}

	mSegmentCount = static_cast<int>(mWireFlat.size()) / 4;
	mPrevWallTouch.assign(static_cast<std::size_t>(mSegmentCount), false);

	char buf[128];
	std::snprintf(
		buf,
		sizeof(buf),
		"Procedural map baked: %.0fx%.0f, %d wall segments.",
		mWidth,
		mHeight,
		mSegmentCount);
	LogManager::getInstance().log(buf);
}

void Map::constrainPlayerCenter(
	float& cx,
	float& cy,
	float worldHalfX,
	float worldHalfY,
	float hitboxHalfW,
	float hitboxHalfH,
	float facingDeg) const
{
	const float maxX = mWidth - worldHalfX;
	const float maxY = mHeight - worldHalfY;
	if (cx < worldHalfX) { cx = worldHalfX; }
	if (cx > maxX) { cx = maxX; }
	if (cy < worldHalfY) { cy = worldHalfY; }
	if (cy > maxY) { cy = maxY; }

	constrainCenterToWireSegments(
		cx,
		cy,
		mWireFlat.data(),
		mSegmentCount,
		hitboxHalfW,
		hitboxHalfH,
		facingDeg);
}

bool Map::pollNewWallContact(
	float cx,
	float cy,
	float hitboxHalfW,
	float hitboxHalfH,
	float facingDeg,
	float touchSlop) const
{
	bool newContact = false;
	for (int s = 0; s < mSegmentCount; ++s)
	{
		const int o = s * 4;
		const float ax = mWireFlat[static_cast<std::size_t>(o + 0)];
		const float ay = mWireFlat[static_cast<std::size_t>(o + 1)];
		const float bx = mWireFlat[static_cast<std::size_t>(o + 2)];
		const float by = mWireFlat[static_cast<std::size_t>(o + 3)];

		float qx = 0.0f;
		float qy = 0.0f;
		closestPointOnSegment2D(cx, cy, ax, ay, bx, by, qx, qy);

		float mx = cx - qx;
		float my = cy - qy;
		float dist = std::sqrt(mx * mx + my * my);
		float ndx = 0.0f;
		float ndy = 0.0f;
		if (dist < 1.0e-4f)
		{
			const float abx = bx - ax;
			const float aby = by - ay;
			const float ablen = std::sqrt(abx * abx + aby * aby);
			if (ablen < 1.0e-6f)
			{
				continue;
			}
			ndx = -aby / ablen;
			ndy = abx / ablen;
		}
		else
		{
			ndx = mx / dist;
			ndy = my / dist;
		}

		const float req = obbExtentAlongInwardNormal(
			ndx, ndy, hitboxHalfW, hitboxHalfH, facingDeg);
		const bool touching = dist > 0.0f && dist <= req + touchSlop;
		if (touching && !mPrevWallTouch[static_cast<std::size_t>(s)])
		{
			newContact = true;
		}
		mPrevWallTouch[static_cast<std::size_t>(s)] = touching;
	}
	return newContact;
}

void Map::drawFloor(Renderer& renderer) const
{
	renderer.drawWorldAxisAlignedQuad(
		mWidth * 0.5f,
		mHeight * 0.5f,
		mWidth * 0.5f,
		mHeight * 0.5f,
		0.34f,
		0.34f,
		0.38f,
		1.0f);
}

void Map::drawWalls(Renderer& renderer) const
{
	if (mSegmentCount < 1 || mWireFlat.empty())
	{
		return;
	}

	renderer.drawWorldLineSegments(
		mWireFlat.data(),
		mSegmentCount,
		0.55f,
		0.55f,
		0.60f,
		0.90f);
}
