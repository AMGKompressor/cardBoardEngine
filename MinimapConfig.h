#pragma once

// =============================================================================
// MinimapConfig — all minimap settings in one place (edit + rebuild).
//
// HOW THE MINIMAP FITS IN THE GAME
// ---------------------------------
// SceneCardBoard creates a Minimap and calls Minimap::draw() each frame after
// the UI, using the same cameraX/cameraY as the health bar. That keeps the
// panel stuck to the screen corner while the world scrolls.
//
// COORDINATE SYSTEM
// -----------------
// Game world uses pixels from top-left (0,0) to Map::width() x Map::height().
// The minimap shrinks the whole level to fit inside panelWidth x panelHeight.
// It does NOT follow the camera — you always see the full layout.
//
// WALLS
// -----
// drawWalls reads Map::wireFlat() — the same line segments as player collision
// and the flashlight mask (from BasicMapLayout_1.h). Change walls in one place,
// minimap + gameplay both update.
//
// ENEMIES
// -------
// Every enemy exists on the level, but a blip is drawn only if distance to the
// player <= enemyRevealRadius. Far enemies are omitted (radar-style reveal).
//
// LIVE TUNING: press ` for ImGui → "Minimap" section (no rebuild needed).
// =============================================================================

struct MinimapConfig
{
	bool enabled = true;

	// --- Panel position on screen ---
	// Renderer HUD trick: world position = camera + offset, so adding camera
	// every frame pins the panel to the viewport corner.
	bool anchorTopRight = true;
	float screenMarginX = 14.0f;  // used when anchorTopRight
	float screenMarginY = 14.0f;
	float screenOffsetX = 14.0f; // used when anchorTopRight is false (top-left)
	float screenOffsetY = 14.0f;

	float panelWidth = 220.0f;
	float panelHeight = 165.0f;
	float innerPadding = 6.0f;   // gap between border and map content
	float borderThickness = 2.0f;

	// --- Enemy radar range (world pixels) ---
	float enemyRevealRadius = 520.0f;

	// --- Wall overlay (Map wire segments) ---
	bool drawWalls = true;
	float wallLineWidth = 1.25f; // reserved for future use; lines use drawWorldLineSegments
	float wallR = 0.55f;
	float wallG = 0.55f;
	float wallB = 0.60f;
	float wallA = 0.90f;

	// --- Panel chrome ---
	float backgroundR = 0.04f;
	float backgroundG = 0.05f;
	float backgroundB = 0.07f;
	float backgroundA = 0.82f;

	float borderR = 0.25f;
	float borderG = 0.70f;
	float borderB = 0.35f;
	float borderA = 0.95f;

	// --- Player blip (true position on full map) ---
	float playerDotRadius = 4.0f;
	float playerR = 0.35f;
	float playerG = 0.85f;
	float playerB = 1.0f;
	float playerA = 1.0f;

	// --- Enemy blip colors by type ---
	float enemyDotRadius = 3.5f;
	float spiritR = 0.55f;
	float spiritG = 0.75f;
	float spiritB = 1.0f;
	float hiddenR = 0.45f;
	float hiddenG = 0.45f;
	float hiddenB = 0.50f;
	float revenantR = 0.95f;
	float revenantG = 0.20f;
	float revenantB = 0.15f;
	float enemyA = 0.95f;
};
