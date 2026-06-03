#pragma once

#ifndef __ROOM_H_
#define __ROOM_H_

#include "BasicMapLayout_1.h"

#include <array>
#include <vector>

class Renderer;

class Room
{
	// MEMBER METHODS
public:
	explicit Room(uint8_t doorways);
	~Room();

	bool Initialise(Renderer& renderer);
	void Process(float deltaTime);
	void Draw(Renderer& renderer);

	template <std::size_t N> void setWallsFromLayout(const std::array<WallSegment, N>& walls);

	void appendWallsToBuffer(std::vector<float>& outBuffer, float offsetX, float offsetY) const;

	float getWidth() const { return mWidth; }
	float getHeight() const { return mHeight; }

	uint8_t getDoorways() const { return mDoorways; }

	void drawWalls(Renderer& renderer, float offsetX, float offsetY) const;
private:
	Room(const Room& room);
	Room& operator=(const Room& room);
	
	// MEMBER DATA
public:
	enum DoorwayFlags : uint8_t
	{
		DOOR_NONE = 0,
		DOOR_NORTH = 1 << 0,
		DOOR_EAST = 1 << 1,
		DOOR_SOUTH = 1 << 2,
		DOOR_WEST = 1 << 3
	};

	static constexpr float kRoomSize = 640.0f;

private:
	float mWidth = kRoomSize;
	float mHeight = kRoomSize;
	uint8_t mDoorways = DOOR_NONE;
	std::vector<WallSegment> mWalls;
};

#endif // __ROOM_H_