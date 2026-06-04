#include "NavGrid.h"

#include "Collision2D_1.h"
#include "Map_1.h"
#include "renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <queue>
#include <utility>

namespace
{
	bool isWorldPointWalkable(
		float worldX,
		float worldY,
		float agentRadius,
		float mapWidth,
		float mapHeight,
		const float* wireFlat,
		int segmentCount)
	{
		if (worldX < agentRadius || worldY < agentRadius
			|| worldX > mapWidth - agentRadius || worldY > mapHeight - agentRadius)
		{
			return false;
		}

		if (wireFlat == nullptr || segmentCount < 1)
		{
			return true;
		}

		for (int s = 0; s < segmentCount; ++s)
		{
			const int o = s * 4;
			const float ax = wireFlat[o + 0];
			const float ay = wireFlat[o + 1];
			const float bx = wireFlat[o + 2];
			const float by = wireFlat[o + 3];

			float qx = 0.0f;
			float qy = 0.0f;
			closestPointOnSegment2D(worldX, worldY, ax, ay, bx, by, qx, qy);

			const float mx = worldX - qx;
			const float my = worldY - qy;
			const float distSq = mx * mx + my * my;
			if (distSq < agentRadius * agentRadius)
			{
				return false;
			}
		}

		return true;
	}
}

bool NavGrid::buildFromMap(const Map& map, float cellSize, float agentRadius)
{
	if (cellSize < 8.0f)
	{
		cellSize = 8.0f;
	}

	mCellSize = cellSize;
	mAgentRadius = agentRadius;
	mMapWidth = map.width();
	mMapHeight = map.height();
	mGridW = static_cast<int>(std::ceil(mMapWidth / mCellSize));
	mGridH = static_cast<int>(std::ceil(mMapHeight / mCellSize));

	if (mGridW < 1 || mGridH < 1)
	{
		mGridW = 0;
		mGridH = 0;
		mWalkable.clear();
		return false;
	}

	mWalkable.assign(static_cast<std::size_t>(mGridW * mGridH), 0);

	const float* wire = map.wireFlat();
	const int segments = map.segmentCount();

	for (int gy = 0; gy < mGridH; ++gy)
	{
		for (int gx = 0; gx < mGridW; ++gx)
		{
			float wx = 0.0f;
			float wy = 0.0f;
			cellToWorld(gx, gy, wx, wy);
			if (isWorldPointWalkable(
					wx, wy, mAgentRadius, mMapWidth, mMapHeight, wire, segments))
			{
				mWalkable[static_cast<std::size_t>(cellIndex(gx, gy))] = 1;
			}
		}
	}

	return true;
}

void NavGrid::worldToCell(float worldX, float worldY, int& outGx, int& outGy) const
{
	outGx = static_cast<int>(std::floor(worldX / mCellSize));
	outGy = static_cast<int>(std::floor(worldY / mCellSize));
	if (outGx < 0) { outGx = 0; }
	if (outGy < 0) { outGy = 0; }
	if (outGx >= mGridW) { outGx = mGridW - 1; }
	if (outGy >= mGridH) { outGy = mGridH - 1; }
}

void NavGrid::cellToWorld(int gx, int gy, float& outX, float& outY) const
{
	outX = (static_cast<float>(gx) + 0.5f) * mCellSize;
	outY = (static_cast<float>(gy) + 0.5f) * mCellSize;
}

int NavGrid::cellIndex(int gx, int gy) const
{
	return gy * mGridW + gx;
}

bool NavGrid::isWalkableCell(int gx, int gy) const
{
	if (gx < 0 || gy < 0 || gx >= mGridW || gy >= mGridH)
	{
		return false;
	}
	return mWalkable[static_cast<std::size_t>(cellIndex(gx, gy))] != 0;
}

bool NavGrid::isWalkableWorld(float worldX, float worldY) const
{
	int gx = 0;
	int gy = 0;
	worldToCell(worldX, worldY, gx, gy);
	return isWalkableCell(gx, gy);
}

bool NavGrid::pickRelocatedPosition(
	float avoidX,
	float avoidY,
	float minDistFromAvoid,
	float fromX,
	float fromY,
	float minDistFromFrom,
	float& outX,
	float& outY) const
{
	if (!isBuilt() || mGridW < 1 || mGridH < 1)
	{
		return false;
	}

	const float avoidSq = minDistFromAvoid * minDistFromAvoid;
	const float fromSq = minDistFromFrom * minDistFromFrom;

	for (int attempt = 0; attempt < 72; ++attempt)
	{
		const int gx = std::rand() % mGridW;
		const int gy = std::rand() % mGridH;
		if (!isWalkableCell(gx, gy))
		{
			continue;
		}

		float wx = 0.0f;
		float wy = 0.0f;
		cellToWorld(gx, gy, wx, wy);

		const float toAvoidX = wx - avoidX;
		const float toAvoidY = wy - avoidY;
		if (toAvoidX * toAvoidX + toAvoidY * toAvoidY < avoidSq)
		{
			continue;
		}

		const float toFromX = wx - fromX;
		const float toFromY = wy - fromY;
		if (toFromX * toFromX + toFromY * toFromY < fromSq)
		{
			continue;
		}

		outX = wx;
		outY = wy;
		return true;
	}

	return false;
}

bool NavGrid::findNearestWalkableCell(int gx, int gy, int& outGx, int& outGy) const
{
	if (isWalkableCell(gx, gy))
	{
		outGx = gx;
		outGy = gy;
		return true;
	}

	const int maxRing = 12;
	for (int ring = 1; ring <= maxRing; ++ring)
	{
		for (int dy = -ring; dy <= ring; ++dy)
		{
			for (int dx = -ring; dx <= ring; ++dx)
			{
				if (std::abs(dx) != ring && std::abs(dy) != ring)
				{
					continue;
				}
				const int nx = gx + dx;
				const int ny = gy + dy;
				if (isWalkableCell(nx, ny))
				{
					outGx = nx;
					outGy = ny;
					return true;
				}
			}
		}
	}

	return false;
}

bool NavGrid::findPath(
	float startX,
	float startY,
	float goalX,
	float goalY,
	std::vector<std::pair<float, float>>& outWaypoints) const
{
	outWaypoints.clear();

	if (!isBuilt())
	{
		return false;
	}

	int startGx = 0;
	int startGy = 0;
	int goalGx = 0;
	int goalGy = 0;
	worldToCell(startX, startY, startGx, startGy);
	worldToCell(goalX, goalY, goalGx, goalGy);

	if (!findNearestWalkableCell(startGx, startGy, startGx, startGy))
	{
		return false;
	}
	if (!findNearestWalkableCell(goalGx, goalGy, goalGx, goalGy))
	{
		return false;
	}

	const int startIdx = cellIndex(startGx, startGy);
	const int goalIdx = cellIndex(goalGx, goalGy);

	if (startIdx == goalIdx)
	{
		float wx = 0.0f;
		float wy = 0.0f;
		cellToWorld(goalGx, goalGy, wx, wy);
		outWaypoints.emplace_back(wx, wy);
		return true;
	}

	const int cellCount = mGridW * mGridH;
	std::vector<int> parent(static_cast<std::size_t>(cellCount), -1);
	std::vector<unsigned char> visited(static_cast<std::size_t>(cellCount), 0);

	std::queue<int> open;
	open.push(startIdx);
	visited[static_cast<std::size_t>(startIdx)] = 1;

	static const int kDx[4] = { 1, -1, 0, 0 };
	static const int kDy[4] = { 0, 0, 1, -1 };

	bool found = false;

	while (!open.empty())
	{
		const int current = open.front();
		open.pop();

		if (current == goalIdx)
		{
			found = true;
			break;
		}

		const int cgx = current % mGridW;
		const int cgy = current / mGridW;

		for (int n = 0; n < 4; ++n)
		{
			const int ngx = cgx + kDx[n];
			const int ngy = cgy + kDy[n];
			if (!isWalkableCell(ngx, ngy))
			{
				continue;
			}

			const int nextIdx = cellIndex(ngx, ngy);
			if (visited[static_cast<std::size_t>(nextIdx)] != 0)
			{
				continue;
			}

			visited[static_cast<std::size_t>(nextIdx)] = 1;
			parent[static_cast<std::size_t>(nextIdx)] = current;
			open.push(nextIdx);
		}
	}

	if (!found)
	{
		return false;
	}

	std::vector<int> reversed;
	for (int at = goalIdx; at != -1; at = parent[static_cast<std::size_t>(at)])
	{
		reversed.push_back(at);
	}

	for (int i = static_cast<int>(reversed.size()) - 1; i >= 0; --i)
	{
		const int idx = reversed[static_cast<std::size_t>(i)];
		const int gx = idx % mGridW;
		const int gy = idx / mGridW;
		float wx = 0.0f;
		float wy = 0.0f;
		cellToWorld(gx, gy, wx, wy);
		outWaypoints.emplace_back(wx, wy);
	}

	return !outWaypoints.empty();
}

void NavGrid::drawDebug(
	Renderer& renderer,
	float cameraX,
	float cameraY,
	float viewWidth,
	float viewHeight) const
{
	if (!isBuilt())
	{
		return;
	}

	const float margin = mCellSize * 2.0f;
	const float minX = cameraX - margin;
	const float maxX = cameraX + viewWidth + margin;
	const float minY = cameraY - margin;
	const float maxY = cameraY + viewHeight + margin;

	int gx0 = static_cast<int>(std::floor(minX / mCellSize));
	int gx1 = static_cast<int>(std::floor(maxX / mCellSize));
	int gy0 = static_cast<int>(std::floor(minY / mCellSize));
	int gy1 = static_cast<int>(std::floor(maxY / mCellSize));

	gx0 = std::max(0, gx0);
	gy0 = std::max(0, gy0);
	gx1 = std::min(mGridW - 1, gx1);
	gy1 = std::min(mGridH - 1, gy1);

	const float nodeHalf = std::max(4.0f, mCellSize * 0.18f);

	for (int gy = gy0; gy <= gy1; ++gy)
	{
		for (int gx = gx0; gx <= gx1; ++gx)
		{
			if (!isWalkableCell(gx, gy))
			{
				continue;
			}

			float wx = 0.0f;
			float wy = 0.0f;
			cellToWorld(gx, gy, wx, wy);
			renderer.drawWorldAxisAlignedQuad(
				wx,
				wy,
				nodeHalf,
				nodeHalf,
				0.15f,
				0.85f,
				0.25f,
				0.45f);
		}
	}
}
