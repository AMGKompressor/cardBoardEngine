// cardBoard — minimal runtime that wires Map + Player + renderer
#pragma once

#include <cstdint>

class Renderer;

namespace CardBoard
{
	class Map;
	class Player;
}

namespace CardBoard
{
	class CardBoardApp
	{
	public:
		static CardBoardApp& getInstance();
		static void destroyInstance();

		bool initialize();
		bool doGameLoop();
		void quit();

	private:
		CardBoardApp();
		~CardBoardApp();

		CardBoardApp(const CardBoardApp&) = delete;
		CardBoardApp& operator=(const CardBoardApp&) = delete;

		void process(float deltaTime);
		void draw();

		void updateCamera();

		static CardBoardApp* sInstance;

		::Renderer* mRenderer = nullptr;
		Map* mMap = nullptr;
		Player* mPlayer = nullptr;

		float mCameraX = 0.0f;
		float mCameraY = 0.0f;
		bool mLooping = true;
		std::int64_t mLastTime = 0;
	};
}
