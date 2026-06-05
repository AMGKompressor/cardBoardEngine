#pragma once

#ifndef __SCENESPLASHSCREEN_H
#define __SCENESPLASHSCREEN_H

#include "Scene.h"

#include <vector>

class Sprite;
class Renderer;
class InputSystem;


class SceneSplashScreen : public Scene
{
	// MEMBER METHODS
public:
	SceneSplashScreen();
	virtual ~SceneSplashScreen();

	virtual bool Initialise(Renderer& renderer);
	virtual void Process(float deltaTime, InputSystem& inputSystem);
	virtual void Draw(Renderer& renderer);
	virtual void DebugDraw();

private:
	SceneSplashScreen(const SceneSplashScreen& sceneSplashScreen);
	SceneSplashScreen& operator=(const SceneSplashScreen& sceneSplashScreen);

	void displaySplashScreen();
	void endDisplaySplashScreens();
	// MEMBER DATA
public:
	
protected:
	Renderer* m_pRenderer;
	std::vector<Sprite*> ss_screens;

	Sprite* currentScreen;

	Sprite* autLogo;
	Sprite* FMODLogo;
	Sprite* openGLLogo;
	Sprite* dearImGUILogo;

	float displayTimer;
	float screenAlpha;
	int splashScreenNumber;
	int splashScreenTotal;

};

#endif // __SCENESPLASHSCREEN_H