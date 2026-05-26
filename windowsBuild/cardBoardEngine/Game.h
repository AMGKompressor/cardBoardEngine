#pragma once

#ifndef __GAME_H
#define __GAME_H

class Renderer;
class Sprite;
class Scene;
class InputSystem;

#include <vector>

class Game
{
	// Member methods
public:
	static Game& GetInstance();
	static void DestroyInstance();

	bool initialise();
	bool DoGameLoop();
	void Quit();

	void ToggleDebugWindow();
	void DebugDraw();

	void setLooping(bool toggle);

	void setScene(int scene);
	int getCurrentScene();
	Scene* getScene(int scene);
	void InitialiseScene(int scene, Renderer& renderer);

	bool isDebugWindowOpen() const { return m_bShowDebugWindow; }
protected:
	void Process(float deltaTime);
	void Draw(Renderer& renderer);

	void ProcessFrameCounting(float deltaTime);

	//void PreloadSprites(Renderer& renderer);

private:
	Game();
	~Game();
	Game(const Game& game);
	Game& operator=(const Game& game);

	// Member data
public:

	bool ingameMusic;
	Renderer* m_pRenderer;
protected:
	static Game* sm_pInstance;

	__int64 m_iLastTime;
	float m_fExecutionTime;
	float m_fElapsedSeconds;
	int m_iFrameCount;
	int m_iFPS;

	bool m_bShowDebugWindow;
	InputSystem* m_pInputSystem;

	std::vector<Scene*> m_scenes;
	int m_iCurrentScene;

#ifdef USE_LAG
	float m_fLag;
	int m_iUpdateCount;
#endif //USE_LAG
	
	bool m_bLooping;

private:


};

#endif // __GAME_H