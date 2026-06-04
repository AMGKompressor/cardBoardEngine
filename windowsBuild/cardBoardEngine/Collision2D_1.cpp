#include "Collision2D_1.h"

#include <cmath>

void orientedRectWorldAabbHalfExtents(
	float halfW,
	float halfH,
	float angleDeg,
	float& outHalfX,
	float& outHalfY)
{
	const float rad = angleDeg * 3.14159265f / 180.0f;
	const float c = std::fabs(std::cos(rad));
	const float s = std::fabs(std::sin(rad));
	outHalfX = c * halfW + s * halfH;
	outHalfY = s * halfW + c * halfH;
}

float obbExtentAlongInwardNormal(
	float nx,
	float ny,
	float localHalfW,
	float localHalfH,
	float angleDeg)
{
	const float rad = angleDeg * 3.14159265f / 180.0f;
	const float c = std::cos(rad);
	const float s = std::sin(rad);
	const float dotW = c * nx - s * ny;
	const float dotH = s * nx + c * ny;
	return localHalfW * std::fabs(dotW) + localHalfH * std::fabs(dotH);
}

void closestPointOnSegment2D(
	float px,
	float py,
	float ax,
	float ay,
	float bx,
	float by,
	float& outQx,
	float& outQy)
{
	const float abx = bx - ax;
	const float aby = by - ay;
	const float apx = px - ax;
	const float apy = py - ay;
	const float ab2 = abx * abx + aby * aby;
	if (ab2 < 1.0e-12f)
	{
		outQx = ax;
		outQy = ay;
		return;
	}
	float t = (apx * abx + apy * aby) / ab2;
	if (t < 0.0f)
	{
		t = 0.0f;
	}
	else if (t > 1.0f)
	{
		t = 1.0f;
	}
	outQx = ax + t * abx;
	outQy = ay + t * aby;
}

void constrainCenterToWireSegments(
	float& px,
	float& py,
	const float* segXY,
	int segmentCount,
	float localHalfW,
	float localHalfH,
	float angleDeg)
{
	if (segXY == nullptr || segmentCount < 1)
	{
		return;
	}
	constexpr int kMaxIters = 32;
	for (int iter = 0; iter < kMaxIters; ++iter)
	{
		float worstMargin = 1.0e30f;
		float pushNx = 0.0f;
		float pushNy = 0.0f;

		for (int s = 0; s < segmentCount; ++s)
		{
			const int o = s * 4;
			const float ax = segXY[o + 0];
			const float ay = segXY[o + 1];
			const float bx = segXY[o + 2];
			const float by = segXY[o + 3];

			float qx = 0.0f;
			float qy = 0.0f;
			closestPointOnSegment2D(px, py, ax, ay, bx, by, qx, qy);

			float mx = px - qx;
			float my = py - qy;
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
				dist = 0.0f;
			}
			else
			{
				ndx = mx / dist;
				ndy = my / dist;
			}

			const float req = obbExtentAlongInwardNormal(ndx, ndy, localHalfW, localHalfH, angleDeg);
			const float margin = dist - req;
			if (margin < worstMargin)
			{
				worstMargin = margin;
				pushNx = ndx;
				pushNy = ndy;
			}
		}

		if (worstMargin >= -0.01f)
		{
			break;
		}
		px += pushNx * (-worstMargin);
		py += pushNy * (-worstMargin);
	}
}

float wrap360(float deg)
{
	while (deg >= 360.0f)
	{
		deg -= 360.0f;
	}
	while (deg < 0.0f)
	{
		deg += 360.0f;
	}
	return deg;
}

float shortestAngleDeltaDegrees(float fromDeg, float toDeg)
{
	float d = toDeg - fromDeg;
	while (d > 180.0f)
	{
		d -= 360.0f;
	}
	while (d < -180.0f)
	{
		d += 360.0f;
	}
	return d;
}

