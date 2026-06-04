#pragma once

// HUD minimap — tune via MinimapConfig.h; live sliders in ImGui (` window).

#include "MinimapConfig.h"

class Renderer;
class Map;
class Player;
class EnemyManager;

class Minimap
{
public:
	const MinimapConfig& config() const { return mConfig; }
	MinimapConfig& config() { return mConfig; }

	void draw(
		Renderer& renderer,
		float cameraX,
		float cameraY,
		float viewWidth,
		float viewHeight,
		const Map& map,
		const Player& player,
		const EnemyManager& enemies) const;

	void debugDraw();

private:
	MinimapConfig mConfig;
};
