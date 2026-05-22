#include "Map.h"

#include "BasicMapLayout.h"
#include "Collision2D.h"

#include "renderer.h"

#include <algorithm>
#include <cmath>

namespace CardBoard
{
	template <std::size_t N>
	void Map::setWallsFromLayout(const std::array<WallSegment, N>& walls)
	{
		static_assert(N * 4u <= 256u, "wire buffer too small");
		mSegmentCount = static_cast<int>(N);
		for (std::size_t i = 0; i < N; ++i)
		{
			mWireFlat[i * 4u + 0u] = walls[i].x0;
			mWireFlat[i * 4u + 1u] = walls[i].y0;
			mWireFlat[i * 4u + 2u] = walls[i].x1;
			mWireFlat[i * 4u + 3u] = walls[i].y1;
		}
		mPrevWallTouch.assign(static_cast<std::size_t>(mSegmentCount), false);
	}

	void Map::loadBasicTutorial()
	{
		mWidth = kDefaultWidth;
		mHeight = kDefaultHeight;
		setWallsFromLayout(BasicMapLayout::kWalls);
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

			const float req = obbExtentAlongInwardNormal(ndx, ndy, hitboxHalfW, hitboxHalfH, facingDeg);
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
		renderer.drawWorldLineSegments(mWireFlat.data(), mSegmentCount, 1.0f, 0.0f, 0.0f, 1.0f);
	}
}
