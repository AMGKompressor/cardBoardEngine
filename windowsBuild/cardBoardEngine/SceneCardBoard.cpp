#include "SceneCardBoard.h"

#include "Map_1.h"
#include "BasicMapLayout_1.h"
#include "Player_1.h"
#include "PlayerConfig_1.h"

#include "logmanager.h"
#include "renderer.h"
#include "sprite.h"

#include "InputSystem.h"
#include "game.h"

#include <SDL.h>

SceneCardBoard::SceneCardBoard()
	: m_pRenderer {0}
	, m_pMap{0}
	, m_pPlayer{0}
	, m_pSceneCardBoard {0}
	, mCameraX {0.0f}
	, mCameraY {0.0f}
	, mLastTime {0}
	, m_pInputSystem{ 0 }
	, mLooping {true}
{

}

SceneCardBoard::~SceneCardBoard() {
	delete m_pPlayer;
	m_pPlayer = nullptr;
	delete m_pMap;
	m_pMap = nullptr;
	delete m_pRenderer;
	m_pRenderer = nullptr;
	delete m_pInputSystem;
	m_pInputSystem = nullptr;
}


bool SceneCardBoard::Initialise(Renderer& renderer) {
	m_pRenderer = &renderer;

	m_pMap = new Map();
	m_pMap->loadBasicTutorial();

	m_pPlayer = new Player();
	PlayerConfig config;
	const float spawnX = (BasicMapLayout::kEntryWest + BasicMapLayout::kEntryEast) * 0.5f;
	const float spawnY = 900.0f;

	if (!m_pPlayer->initialize(*m_pRenderer, config, spawnX, spawnY))
	{
		LogManager::getInstance().log("cardBoard: player init failed.");
		return false;
	}

	mLastTime = SDL_GetPerformanceCounter();
	updateCamera();
	m_pRenderer->setCamera(mCameraX, mCameraY);

	LogManager::getInstance().log("cardBoard engine started (WASD, Shift sprint, LMB toggle flashlight, H debug).");
	return true;
}

void SceneCardBoard::updateCamera() {
	if (m_pRenderer == nullptr || m_pPlayer == nullptr || m_pMap == nullptr)
	{
		return;
	}

	const float viewW = static_cast<float>(m_pRenderer->getWidth());
	const float viewH = static_cast<float>(m_pRenderer->getHeight());

	float maxCamX = m_pMap->width() - viewW;
	float maxCamY = m_pMap->height() - viewH;
	if (maxCamX < 0.0f) { maxCamX = 0.0f; }
	if (maxCamY < 0.0f) { maxCamY = 0.0f; }

	mCameraX = m_pPlayer->x() - viewW * 0.5f;
	mCameraY = m_pPlayer->y() - viewH * 0.5f;

	if (mCameraX < 0.0f) { mCameraX = 0.0f; }
	if (mCameraY < 0.0f) { mCameraY = 0.0f; }
	if (mCameraX > maxCamX) { mCameraX = maxCamX; }
	if (mCameraY > maxCamY) { mCameraY = maxCamY; }
}

void SceneCardBoard::Process(float deltaTime, InputSystem& inputSystem) {

	if (inputSystem.GetKeyState(SDL_SCANCODE_ESCAPE) == BS_PRESSED) {
		Game::GetInstance().Quit();
		return;
	}

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
	const bool sprintHeld = keys[SDL_SCANCODE_LSHIFT] != 0u || keys[SDL_SCANCODE_RSHIFT] != 0u;

	m_pPlayer->update(
		deltaTime,
		*m_pMap,
		mCameraX,
		mCameraY,
		keys[SDL_SCANCODE_W] != 0,
		keys[SDL_SCANCODE_A] != 0,
		keys[SDL_SCANCODE_S] != 0,
		keys[SDL_SCANCODE_D] != 0,
		sprintHeld);

	updateCamera();
	m_pRenderer->setCamera(mCameraX, mCameraY);
}

void SceneCardBoard::Draw(Renderer& renderer) {
	m_pRenderer->clear();
	m_pMap->drawFloor(*m_pRenderer);
	m_pMap->drawWalls(*m_pRenderer);
	m_pPlayer->drawFlashlightMask(*m_pRenderer, *m_pMap, mCameraX, mCameraY);
	m_pPlayer->drawNoisePulses(*m_pRenderer);
	m_pPlayer->drawSprite(*m_pRenderer);
	m_pPlayer->drawHitboxDebug(*m_pRenderer);
	m_pRenderer->present();
}

void SceneCardBoard::DebugDraw() {
	;
}

