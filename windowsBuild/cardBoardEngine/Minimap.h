#pragma once

// Minimap — zoomed radar (follows player) or full-map overview. See MinimapConfig.h.
//
// Owned by SceneCardBoard (m_pMinimap). Settings live in MinimapConfig.h.
// Draw is called from SceneCardBoard::Draw after the UI meters.

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

	// Renders the panel using world-space quads/lines + camera offset (HUD style).
	void draw(
		Renderer& renderer,
		float cameraX,
		float cameraY,
		float viewWidth,
		float viewHeight,
		const Map& map,
		const Player& player,
		const EnemyManager& enemies) const;

	// ImGui sliders in SceneCardBoard::DebugDraw (` key).
	void debugDraw();

private:
	MinimapConfig mConfig;
};
