// cardBoard engine — 2D segment collision helpers
#pragma once

namespace CardBoard
{
	void orientedRectWorldAabbHalfExtents(
		float halfW,
		float halfH,
		float angleDeg,
		float& outHalfX,
		float& outHalfY);

	float obbExtentAlongInwardNormal(
		float nx,
		float ny,
		float localHalfW,
		float localHalfH,
		float angleDeg);

	void closestPointOnSegment2D(
		float px,
		float py,
		float ax,
		float ay,
		float bx,
		float by,
		float& outQx,
		float& outQy);

	void constrainCenterToWireSegments(
		float& px,
		float& py,
		const float* segXY,
		int segmentCount,
		float localHalfW,
		float localHalfH,
		float angleDeg);

	float wrap360(float deg);
	float shortestAngleDeltaDegrees(float fromDeg, float toDeg);
}
