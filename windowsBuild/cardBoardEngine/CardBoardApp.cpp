#include "CardBoardApp.h"

#include "Map.h"
#include "BasicMapLayout.h"
#include "Player.h"
#include "PlayerConfig.h"

#include "logmanager.h"
#include "renderer.h"
#include "sprite.h"

#include <SDL.h>

namespace CardBoard
{
	CardBoardApp* CardBoardApp::sInstance = nullptr;

	CardBoardApp& CardBoardApp::getInstance()
	{
		if (sInstance == nullptr)
		{
			sInstance = new CardBoardApp();
		}
		return *sInstance;
	}

	void CardBoardApp::destroyInstance()
	{
		delete sInstance;
		sInstance = nullptr;
	}

	CardBoardApp::CardBoardApp() = default;

	CardBoardApp::~CardBoardApp()
	{
		delete mPlayer;
		mPlayer = nullptr;
		delete mMap;
		mMap = nullptr;
		delete mRenderer;
		mRenderer = nullptr;
	}

	bool CardBoardApp::initialize()
	{
		mRenderer = new Renderer();
		if (!mRenderer->initialize(true, 1280, 720))
		{
			LogManager::getInstance().log("cardBoard: renderer init failed.");
			return false;
		}

		mMap = new Map();
		mMap->loadBasicTutorial();

		mPlayer = new Player();
		PlayerConfig config;
		const float spawnX =
			(BasicMapLayout::kEntryWest + BasicMapLayout::kEntryEast) * 0.5f;
		const float spawnY = 900.0f;
		if (!mPlayer->initialize(*mRenderer, config, spawnX, spawnY))
		{
			LogManager::getInstance().log("cardBoard: player init failed.");
			return false;
		}

		mLastTime = SDL_GetPerformanceCounter();
		updateCamera();
		mRenderer->setCamera(mCameraX, mCameraY);

		LogManager::getInstance().log("cardBoard engine started (WASD, Shift sprint, LMB toggle flashlight, H debug).");
		return true;
	}

	void CardBoardApp::quit()
	{
		mLooping = false;
	}

	void CardBoardApp::updateCamera()
	{
		if (mRenderer == nullptr || mPlayer == nullptr || mMap == nullptr)
		{
			return;
		}

		const float viewW = static_cast<float>(mRenderer->getWidth());
		const float viewH = static_cast<float>(mRenderer->getHeight());

		float maxCamX = mMap->width() - viewW;
		float maxCamY = mMap->height() - viewH;
		if (maxCamX < 0.0f) { maxCamX = 0.0f; }
		if (maxCamY < 0.0f) { maxCamY = 0.0f; }

		mCameraX = mPlayer->x() - viewW * 0.5f;
		mCameraY = mPlayer->y() - viewH * 0.5f;

		if (mCameraX < 0.0f) { mCameraX = 0.0f; }
		if (mCameraY < 0.0f) { mCameraY = 0.0f; }
		if (mCameraX > maxCamX) { mCameraX = maxCamX; }
		if (mCameraY > maxCamY) { mCameraY = maxCamY; }
	}

	void CardBoardApp::process(float deltaTime)
	{
		if (deltaTime < 0.0f)
		{
			deltaTime = 0.0f;
		}
		else if (deltaTime > 0.1f)
		{
			deltaTime = 0.1f;
		}

		SDL_PumpEvents();
		const Uint8* keys = SDL_GetKeyboardState(nullptr);
		const bool sprintHeld =
			keys[SDL_SCANCODE_LSHIFT] != 0u || keys[SDL_SCANCODE_RSHIFT] != 0u;

		mPlayer->update(
			deltaTime,
			*mMap,
			mCameraX,
			mCameraY,
			keys[SDL_SCANCODE_W] != 0,
			keys[SDL_SCANCODE_A] != 0,
			keys[SDL_SCANCODE_S] != 0,
			keys[SDL_SCANCODE_D] != 0,
			sprintHeld);

		updateCamera();
		mRenderer->setCamera(mCameraX, mCameraY);
	}

	void CardBoardApp::draw()
	{
		mRenderer->clear();
		mMap->drawFloor(*mRenderer);
		mMap->drawWalls(*mRenderer);
		mPlayer->drawFlashlightMask(*mRenderer, *mMap, mCameraX, mCameraY);
		mPlayer->drawNoisePulses(*mRenderer);
		mPlayer->drawSprite(*mRenderer);
		mPlayer->drawHitboxDebug(*mRenderer);
		mRenderer->present();
	}

	bool CardBoardApp::doGameLoop()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				quit();
			}
			else if (event.type == SDL_KEYDOWN && event.key.repeat == 0)
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				{
					quit();
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_H)
				{
					mPlayer->setShowHitboxDebug(!mPlayer->showHitboxDebug());
				}
			}
			else if (
				event.type == SDL_MOUSEBUTTONDOWN
				&& event.button.button == SDL_BUTTON_LEFT
				&& event.button.clicks == 1)
			{
				mPlayer->toggleFlashlight();
			}
		}

		if (mLooping)
		{
			const Uint64 now = SDL_GetPerformanceCounter();
			const float deltaTime =
				(now - mLastTime) / static_cast<float>(SDL_GetPerformanceFrequency());
			mLastTime = static_cast<std::int64_t>(now);

			process(deltaTime);
			draw();
		}

		return mLooping;
	}
}
