#pragma once

#ifndef __MOUSECURSOR_H
#define __MOUSECURSOR_H

class Renderer;
class Sprite;


class MouseCursor
{
	// MEMBER METHODS
public:

	MouseCursor();
	~MouseCursor();

	bool Initialise(Renderer& renderer);
	void Process(float deltaTime, int x, int y);
	void Draw(Renderer& renderer);

protected:
	void setCursorCoords(int mx, int my);

private:
	MouseCursor(const MouseCursor& mouseCursor);
	MouseCursor& operator=(const MouseCursor& mouseCursor);

	// MEMBER DATA
public:
	
protected:
	Sprite* cursor_sprite;
	int cursorX;
	int cursorY;
};

#endif // __MOUSECURSOR_H