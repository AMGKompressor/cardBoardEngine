#pragma once
#ifndef __SCENEPAUSE_H_
#define __SCENEPAUSE_H_

#include "Scene.h"

class Sprite;
class Renderer;
class InputSystem;

class ScenePause : public Scene
{
	// MEMBER METHODS
public:
	ScenePause();
	virtual ~ScenePause();

	virtual bool Initialise(Renderer& renderer);
	virtual void Process(float deltaTime, InputSystem& inputSystem);
	virtual void Draw(Renderer& renderer);

	virtual void DebugDraw();
protected:

private:
	ScenePause(const ScenePause& scenePause);
	ScenePause& operator=(const ScenePause& scenePause);

	// MEMBER DATA
public:

protected:
	Sprite* mPauseText;
	Sprite* mPauseResumePrompt;
	Sprite* mPauseQuitPrompt;

private:
	Node* resumeNode;
	Node* quitNode;

	float menuFlashTimer;
	bool menuShowPrompt;
};

#endif // __SCENEPAUSE_H_