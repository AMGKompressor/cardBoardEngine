#include "SceneSplashScreen.h"

#include "Renderer.h"
#include "Sprite.h"

#include "Game.h"

#include <cassert>

#include <iostream>

SceneSplashScreen::SceneSplashScreen()
	: autLogo(nullptr), FMODLogo(nullptr), currentScreen(nullptr),
	m_pRenderer(nullptr), screenAlpha(1.0f), displayTimer(0.0f),
	splashScreenNumber(1), splashScreenTotal(2)
{

}

SceneSplashScreen::~SceneSplashScreen()
{
	autLogo = nullptr;
	FMODLogo = nullptr;
	currentScreen = nullptr;
}

bool SceneSplashScreen::Initialise(Renderer& renderer)
{
	// store renderer
	m_pRenderer = &renderer;

	// generate splashscreen sprites
	autLogo = renderer.createSprite("../assets/splashscreens/aut_logo.png");
	FMODLogo = renderer.createSprite("../assets/splashscreens/FMOD_Logo.png");
	openGLLogo = renderer.createSprite("../assets/splashscreens/openGL_splashscreen.png");
	dearImGUILogo = renderer.createSprite("../assets/splashscreens/DearImGui_splashscreen.png");
	
	autLogo->setScale(0.2f);

	// set current splash screen and total
	screenAlpha = 1.0f;
	splashScreenNumber = 1;
	splashScreenTotal = 4;

	currentScreen = autLogo;
	
	currentScreen->setAlpha(screenAlpha);

	return true;
}

void SceneSplashScreen::Process(float deltaTime, InputSystem& inputSystem)
{
	if (currentScreen == nullptr) return;

	// update splashscreen timer
	displayTimer += deltaTime;

	// adjust transparency 
	screenAlpha = 1.0f - (displayTimer / 1.25f);

	// check if splashscreen is faded
	if (screenAlpha <= 0.0f)
	{
		// change splashscreen
		screenAlpha = 0.0f;
		displayTimer = 0.0f;
		endDisplaySplashScreens();
	}
	else
	{
		currentScreen->setAlpha(screenAlpha);
	}

	currentScreen->process(deltaTime);
}

void SceneSplashScreen::Draw(Renderer& renderer)
{
	if (currentScreen == nullptr) return;

	currentScreen->setX(renderer.getWidth() / 2);
	currentScreen->setY(renderer.getHeight() / 2);
	currentScreen->draw(renderer);
}

void SceneSplashScreen::DebugDraw()
{

}

void SceneSplashScreen::endDisplaySplashScreens()
{
	splashScreenNumber++;

	if (splashScreenNumber > splashScreenTotal)
	{
		// go to main menu
		Game::GetInstance().InitialiseScene(1, *m_pRenderer);
		Game::GetInstance().setScene(1);
		return;
	}

	// reset alpha
	screenAlpha = 1.0f;

	// move along splashscreens
	switch (splashScreenNumber)
	{
	case 1: currentScreen = autLogo;  break;
	case 2: currentScreen = FMODLogo; break;
	case 3: currentScreen = openGLLogo; break;
	case 4: currentScreen = dearImGUILogo; break;
	}

	if (currentScreen != nullptr)
		currentScreen->setAlpha(screenAlpha);
}