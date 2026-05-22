// cardBoard — world bounds, wall segments, collision, and wall drawing
#pragma once

#include "BasicMapLayout.h"

#include <array>
#include <vector>

class Renderer;

namespace CardBoard
{
	class Map
	{
	public:
		static constexpr float kDefaultWidth = 2560.0f;
		static constexpr float kDefaultHeight = 1920.0f;

		void loadBasicTutorial();

		float width() const { return mWidth; }
		float height() const { return mHeight; }
		const float* wireFlat() const { return mWireFlat.data(); }
		int segmentCount() const { return mSegmentCount; }

		void constrainPlayerCenter(
			float& cx,
			float& cy,
			float worldHalfX,
			float worldHalfY,
			float hitboxHalfW,
			float hitboxHalfH,
			float facingDeg) const;

		bool pollNewWallContact(
			float cx,
			float cy,
			float hitboxHalfW,
			float hitboxHalfH,
			float facingDeg,
			float touchSlop = 3.0f) const;

		void drawWalls(Renderer& renderer) const;
		void drawFloor(Renderer& renderer) const;

	private:
		template <std::size_t N>
		void setWallsFromLayout(const std::array<WallSegment, N>& walls);

		float mWidth = kDefaultWidth;
		float mHeight = kDefaultHeight;
		std::array<float, 256> mWireFlat{};
		int mSegmentCount = 0;
		mutable std::vector<bool> mPrevWallTouch;
	};
}
