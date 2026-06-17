#pragma once

#ifndef __SCENE_H_
#define __SCENE_H_

#include <string>

class Renderer;
class InputSystem;
class Sprite;
class Game;

struct Node;

struct Node
{
	bool selected;
	Sprite* sprite;
	std::string text;

	int x;
	int y;

	struct Node* next;
	struct Node* prev;
};

class Scene
{
	// Member Methods
public:
	Scene();
	virtual ~Scene();

	virtual bool Initialise(Renderer& renderer) = 0;
	virtual void Process(float deltaTime, InputSystem& inputSystem) = 0;
	virtual void Draw(Renderer& renderer) = 0;

	virtual void DebugDraw() = 0;

protected:
	int CENTER_X;

	Node* insertAtEnd(Node* head);
	bool searchList(Node* start, bool search);

	Node* moveUp(Node* current);
	Node* moveDown(Node* current);
private:
	Scene(const Scene& scene);
	Scene& operator=(const Scene& scene);

	// Member data
public:

protected:
	Node* head;

	Sprite* selectArrow;
	Node* currentSelection;
};

#endif // !__SCENE_