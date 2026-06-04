#pragma once

#include <vector>

class Map;
class Renderer;

// Grid built from Map wall segments. BFS finds a path of cell centers around walls.
class NavGrid
{
public:
	bool buildFromMap(const Map& map, float cellSize = 48.0f, float agentRadius = 36.0f);

	bool isBuilt() const { return mGridW > 0 && mGridH > 0; }
	int gridWidth() const { return mGridW; }
	int gridHeight() const { return mGridH; }
	float cellSize() const { return mCellSize; }

	bool isWalkableWorld(float worldX, float worldY) const;

	// For Hidden respawn after stun — random cell away from player.
	bool pickRelocatedPosition(
		float avoidX,
		float avoidY,
		float minDistFromAvoid,
		float fromX,
		float fromY,
		float minDistFromFrom,
		float& outX,
		float& outY) const;
	bool findPath(
		float startX,
		float startY,
		float goalX,
		float goalY,
		std::vector<std::pair<float, float>>& outWaypoints) const;

	void worldToCell(float worldX, float worldY, int& outGx, int& outGy) const;
	void cellToWorld(int gx, int gy, float& outX, float& outY) const;

	// Walkable BFS nodes in view (green dots). Toggle with H in SceneCardBoard.
	void drawDebug(
		Renderer& renderer,
		float cameraX,
		float cameraY,
		float viewWidth,
		float viewHeight) const;

private:
	bool isWalkableCell(int gx, int gy) const;
	int cellIndex(int gx, int gy) const;
	bool findNearestWalkableCell(int gx, int gy, int& outGx, int& outGy) const;

	float mCellSize = 48.0f;
	float mAgentRadius = 36.0f;
	float mMapWidth = 0.0f;
	float mMapHeight = 0.0f;
	int mGridW = 0;
	int mGridH = 0;
	std::vector<unsigned char> mWalkable;
};
