// =============================================================================
// Map_1.cpp — level geometry, wall collision, and drawing
// =============================================================================
//
// HOW TO BUILD A NEW LEVEL (for level designers / programmers)
// -----------------------------------------------------------------------------
// 1. Open BasicMapLayout_1.h (or copy it to e.g. Level2Layout.h).
// 2. Add wall lines to kWallData[] using Hor(), Ver(), or raw {x0,y0,x1,y1}.
//    Coordinates are in WORLD PIXELS. Origin (0,0) is TOP-LEFT; +Y goes DOWN.
// 3. Add a new loader here, e.g. loadLevel2(), that calls setWallsFromLayout(...).
// 4. Call that loader from SceneCardBoard instead of loadBasicTutorial().
//
// There is NO tilemap: walls are line segments only. What you draw in drawWalls()
// is exactly what collision uses — same mWireFlat[] data.
//
// LIMITS: at most 64 wall segments (256 floats in mWireFlat). World size defaults
// to 2560 x 1920 (Map::kDefaultWidth / kDefaultHeight).
//
// COLLISION MATH lives mostly in Collision2D_1.cpp (constrainCenterToWireSegments,
// closestPointOnSegment2D, obbExtentAlongInwardNormal). This file loads segments
// and calls those helpers.
// =============================================================================

#include "Map_1.h"

#include "BasicMapLayout_1.h"
#include "Collision2D_1.h"

#include "renderer.h"

#include <algorithm>
#include <cmath>

// -----------------------------------------------------------------------------
// setWallsFromLayout — copy level wall list into the runtime "wire" buffer
// -----------------------------------------------------------------------------
// Input:  array of WallSegment { x0, y0, x1, y1 } from a layout header.
// Output: mWireFlat[] packed as [x0,y0,x1,y1, x0,y0,x1,y1, ...] per segment.
//         mSegmentCount = number of walls.
//
// Every system that cares about walls reads mWireFlat:
//   - constrainPlayerCenter() / enemies — solid collision
//   - pollNewWallContact() — footstep noise when you bump a wall
//   - Renderer flashlight mask — light blocked by same segments
//   - drawWalls() — red debug/outline lines
// -----------------------------------------------------------------------------
template <std::size_t N>
void Map::setWallsFromLayout(const std::array<WallSegment, N>& walls)
{
	static_assert(N * 4u <= 256u, "wire buffer too small"); // max 64 segments

	mSegmentCount = static_cast<int>(N);
	for (std::size_t i = 0; i < N; ++i)
	{
		mWireFlat[i * 4u + 0u] = walls[i].x0;
		mWireFlat[i * 4u + 1u] = walls[i].y0;
		mWireFlat[i * 4u + 2u] = walls[i].x1;
		mWireFlat[i * 4u + 3u] = walls[i].y1;
	}

	// One flag per segment: was the player touching it last frame?
	// Used by pollNewWallContact() to detect NEW bumps (not continuous slide).
	mPrevWallTouch.assign(static_cast<std::size_t>(mSegmentCount), false);
}

// -----------------------------------------------------------------------------
// loadBasicTutorial — load the default hardcoded level from BasicMapLayout_1.h
// -----------------------------------------------------------------------------
// To add another level: duplicate this pattern:
//   void Map::loadWarehouse() {
//       mWidth = 3000.f;   // optional: different world size
//       mHeight = 2400.f;
//       setWallsFromLayout(WarehouseLayout::kWalls);
//   }
// -----------------------------------------------------------------------------
void Map::loadBasicTutorial()
{
	mWidth = kDefaultWidth;
	mHeight = kDefaultHeight;
	setWallsFromLayout(BasicMapLayout::kWalls);
}

// -----------------------------------------------------------------------------
// constrainPlayerCenter — keep a moving entity inside the map and outside walls
// -----------------------------------------------------------------------------
// Called every frame from Player_1::update() (and Enemy movement) AFTER computing
// a desired position (cx, cy). Parameters are passed by reference: this function
// MODIFIES cx/cy in place to the corrected position.
//
// Parameters:
//   cx, cy           — entity center (usually player mX, mY)
//   worldHalfX/Y     — half-size of axis-aligned bounds for MAP EDGES only
//                      (from orientedRectWorldAabbHalfExtents on the hitbox)
//   hitboxHalfW/H    — half-width/height of the entity's oriented box (OBB)
//   facingDeg        — rotation of that box (player aim direction)
//
// Step A: clamp to world rectangle so the outer AABB stays inside [0..width/height].
// Step B: constrainCenterToWireSegments() pushes the center away from each wall
//         line until the OBB no longer penetrates. Iterates in Collision2D_1.cpp.
// -----------------------------------------------------------------------------
void Map::constrainPlayerCenter(
	float& cx,
	float& cy,
	float worldHalfX,
	float worldHalfY,
	float hitboxHalfW,
	float hitboxHalfH,
	float facingDeg) const
{
	// --- Step A: world bounds (invisible outer box of the level) ---
	const float maxX = mWidth - worldHalfX;
	const float maxY = mHeight - worldHalfY;
	if (cx < worldHalfX) { cx = worldHalfX; }
	if (cx > maxX) { cx = maxX; }
	if (cy < worldHalfY) { cy = worldHalfY; }
	if (cy > maxY) { cy = maxY; }

	// --- Step B: wall segments (see Collision2D_1.cpp for push logic) ---
	constrainCenterToWireSegments(
		cx,
		cy,
		mWireFlat.data(),
		mSegmentCount,
		hitboxHalfW,
		hitboxHalfH,
		facingDeg);
}

// -----------------------------------------------------------------------------
// pollNewWallContact — did the entity JUST touch a wall this frame?
// -----------------------------------------------------------------------------
// Returns true once per segment when contact begins (not every frame while sliding).
// Player uses this to play footstep / bump noise pulses.
//
// For each wall segment A→B:
//   1. Find closest point Q on the segment to center (cx, cy).
//   2. dist = distance from center to Q; normal = direction from Q to center.
//   3. req = how far the OBB extends along that normal (depends on facingDeg).
//   4. "touching" if dist <= req + touchSlop (small fudge, default 3px).
//   5. If touching now but mPrevWallTouch[s] was false → newContact = true.
// -----------------------------------------------------------------------------
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
			// Center is exactly on the wall line: pick a perpendicular normal.
			const float abx = bx - ax;
			const float aby = by - ay;
			const float ablen = std::sqrt(abx * abx + aby * aby);
			if (ablen < 1.0e-6f)
			{
				continue; // degenerate zero-length segment — skip
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

// -----------------------------------------------------------------------------
// drawFloor — single grey rectangle under everything (not collision geometry)
// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
// drawWalls — draw each segment as a red line (visual only; collision uses same data)
// -----------------------------------------------------------------------------
// Tip for level building: run the game and align coordinates until red lines match
// the corridors you want. Gaps between segment endpoints = doorways (no collision).
// -----------------------------------------------------------------------------
void Map::drawWalls(Renderer& renderer) const
{
	renderer.drawWorldLineSegments(mWireFlat.data(), mSegmentCount, 1.0f, 0.0f, 0.0f, 1.0f);
}
