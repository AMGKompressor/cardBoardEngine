#include "game.h"

#include "renderer.h"
#include "logmanager.h"
#include "Sprite.h"

#include "InputSystem.h"`

#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "SoundSystem.h"

#include "SceneCardBoard.h"

#include <iostream>


Game* Game::sm_pInstance = 0;

Game& Game::GetInstance()
{
	if (sm_pInstance == 0)
	{
		sm_pInstance = new Game();
	}

	return (*sm_pInstance);
}

void Game::DestroyInstance()
{
	delete sm_pInstance;
	sm_pInstance = 0;
}

Game::Game() :	m_pRenderer(0),m_pInputSystem(0), m_bLooping(true), m_bShowDebugWindow(false)  
				, m_iCurrentScene(0), ingameMusic(true)
{

}

Game::~Game()
{
	delete m_pRenderer;
	m_pRenderer = 0;

	delete m_pInputSystem;
	m_pInputSystem = 0;

	delete m_pSoundSystem;
	m_pSoundSystem = 0;

	for (Scene* scene : m_scenes)
	{
		delete scene;
	}
	m_scenes.clear();

	SoundSystem::GetInstance().Shutdown();
}

void Game::Quit()
{
	m_bLooping = false;
}

bool Game::initialise()
{
	int bbWidth = 1280;
	int bbHeight = 720;

	m_pRenderer = new Renderer();
	if (!m_pRenderer->initialize(true, bbWidth, bbHeight))
	{
		LogManager::getInstance().log("Renderer failed to initialise!");
		return false;
	}

	bbWidth = m_pRenderer->getWidth();
	bbHeight = m_pRenderer->getHeight();

	m_iLastTime = SDL_GetPerformanceCounter();

	m_pRenderer->setClearColor(0, 0, 0);

	m_pInputSystem = new InputSystem();

	if (!m_pInputSystem->Initialise())
	{
		LogManager::getInstance().log("InputSystem failed to initialise!");
		return false;
	}
	
	SoundSystem::GetInstance().SetMasterVolume(0.9f);



	Scene* pCardBoardScene = new SceneCardBoard();
	if (!pCardBoardScene->Initialise(*m_pRenderer)) {
		LogManager::getInstance().log("Failed to init CardBoard Scene");
		return false;
	}
	m_scenes.push_back(pCardBoardScene);

	m_iCurrentScene = 0;


	return true;
}

bool Game::DoGameLoop()
{
	const float stepSize = 1.0f / 60.0f;

	m_pInputSystem->ProcessInput();

	if (m_bLooping)
	{
		Uint64 current = SDL_GetPerformanceCounter();
		float deltaTime = (current - m_iLastTime) / static_cast<float>(SDL_GetPerformanceFrequency());

		m_iLastTime = current;
		m_fExecutionTime += deltaTime;

		Process(deltaTime);

#ifdef USE_LAG
		m_fLag += deltaTime;

		int innerLag = 0;
		
		while (m_fLag >= stepSize)
		{
			Process(stepSize);
			m_fLag -= stepSize;

			++m_iUpdateCount;
			++innerLag;
		}
#endif // USE_LAG

		Draw(*m_pRenderer);

	}

	return m_bLooping;
}

void Game::Process(float deltaTime)
{
	ProcessFrameCounting(deltaTime);

	// TODO: Add game eobjects to process here!

	m_scenes[m_iCurrentScene]->Process(deltaTime, *m_pInputSystem);
}

void Game::Draw(Renderer& renderer)
{
	++m_iFrameCount;

	renderer.clear();

	// TODO: Add game objects to draw here!
    m_scenes[m_iCurrentScene]->Draw(renderer);

	renderer.present();
}

void Game::ProcessFrameCounting(float deltaTime)
{
	// Count total simulation time elapsed:
	m_fElapsedSeconds += deltaTime;

	// Frame Counter:

	if (m_fElapsedSeconds > 1.0f)
	{
		m_fElapsedSeconds -= 1.0f;
		m_iFPS = m_iFrameCount;
		m_iFrameCount = 0;
	}
}

void Game::DebugDraw
()
{
	if (m_bShowDebugWindow)
	{
		bool open = true;
		ImGui::Begin("Debug Window", &open, ImGuiWindowFlags_MenuBar);
		ImGui::Text("COMP710 GP Framework (%s)", "2024, S2");
		if (ImGui::Button("Quit"))
		{
			Quit();
		}
		ImGui::SliderInt("Active scene", &m_iCurrentScene, 0, m_scenes.size() - 1, "%d");
		m_scenes[m_iCurrentScene]->DebugDraw();
		ImGui::End();
	}
}


void Game::ToggleDebugWindow
()
{
	m_bShowDebugWindow = !m_bShowDebugWindow;
	m_pInputSystem->ShowMouseCursor(m_bShowDebugWindow);
}

void Game::setLooping(bool toggle)
{
	m_bLooping = toggle;
}

Scene* Game::getScene(int scene)
{
	if (scene >= 0 && scene < (int)m_scenes.size())
	{
		return m_scenes[scene];
	}
	return nullptr;
}

void Game::setScene(int scene)
{
	if (scene >= 0 && scene < (int)m_scenes.size())
	{
		m_iCurrentScene = scene;
	}
}

int Game::getCurrentScene()
{
	return m_iCurrentScene;
}

void Game::InitialiseScene(int scene, Renderer& renderer)
{
	if (scene >= 0 && scene < (int)m_scenes.size())
	{
		m_scenes[scene]->Initialise(renderer);
		m_iCurrentScene = scene;
	}
}
