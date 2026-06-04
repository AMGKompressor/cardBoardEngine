#include "MouseCursor.h"

#include "Sprite.h"
#include "Renderer.h"

MouseCursor::MouseCursor()
{
	
}

MouseCursor::~MouseCursor()
{
	cursor_sprite = nullptr;
	cursorX = 0;
	cursorY = 0;
}

bool MouseCursor::Initialise(Renderer& renderer)
{
	cursor_sprite = renderer.createSprite("textures/cursor.png");
	cursorX = 200;
	cursorY = 200;
	cursor_sprite->setScale(0.05f);

	return true;
}

void MouseCursor::Process(float deltaTime, int x, int y)
{
	setCursorCoords(x, y);
	cursor_sprite->process(deltaTime);
}

void MouseCursor::Draw(Renderer& renderer)
{
	cursor_sprite->draw(renderer);
}

void MouseCursor::setCursorCoords(int mx, int my)
{
	cursor_sprite->setX(mx);
	cursor_sprite->setY(my);
}