#pragma once

#ifndef __UI_H
#define __UI_H

class PlayerConfig;
class Renderer;
class Sprite;
class Player;

class UI
{
	// MEMBER METHODS
public:
	UI();
	~UI();

	bool initialise(Renderer& renderer, Player* player, PlayerConfig* config);
	void process(float deltaTime);
	void draw(Renderer& renderer);

	// SANITY
	void drawSanityMeter(Renderer& renderer, float cameraX, float cameraY);
	bool noSanity(float deltaTime);
	void adjustSanity(float deltaTime);
	float sanityRatio();

	// HEALTH
	void drawHealthMeter(Renderer& renderer, float cameraX, float cameraY);
	void adjustHealth(float deltaTime);
	float healthRatio();
protected:
	
private:
	UI(const UI& ui);
	UI& operator=(const UI& ui);

	// MEMBER DATA
public:
	Player* ui_player;
protected:
	PlayerConfig* mConfig;

};

#endif // __UI_H