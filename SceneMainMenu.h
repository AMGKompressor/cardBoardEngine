#pragma once
#ifndef __SCENEMAINMENU_H_
#define __SCENEMAINMENU_H_

#include "Scene.h"

class Sprite;
class Renderer;
class InputSystem;

class SceneMainMenu : public Scene
{
	// MEMBER METHODS
public:
	SceneMainMenu();
	virtual ~SceneMainMenu();

	virtual bool Initialise(Renderer& renderer);
	virtual void Process(float deltaTime, InputSystem& inputSystem);
	virtual void Draw(Renderer& renderer);
	virtual void DebugDraw();

private:
	SceneMainMenu(const SceneMainMenu& sceneMainMenu);
	SceneMainMenu& operator=(const SceneMainMenu& sceneMainMenu);
	// MEMBER DATA
public:

protected:
	Renderer* m_pRenderer;
	Node* startNode;
	Node* optionsNode;
	Node* instructionNode;
	Node* quitNode;
private:
	Sprite* menuTitle;
	Sprite* menuStartPrompt;
	Sprite* menuOptionPrompt;
	Sprite* menuInstructionPrompt;
	Sprite* menuQuitPrompt;

	float menuflashTimer;
	bool menuShowPrompt;
};

#endif // __SCENEMAINMENU_H_