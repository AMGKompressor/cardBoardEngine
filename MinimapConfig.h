#pragma once

// ---------------------------------------------------------------------------
// Minimap tuning — edit these values and rebuild (no code changes needed).
//
// enemyRevealRadius: only enemies within this distance (world pixels) of the
//   player appear as blips. Increase to see threats farther away.
//
// drawWalls: draws the same wall segments as gameplay collision (Map wire).
//   Set false for a cleaner map with only player/enemy dots.
//
// Panel position: anchorTopRight places the map in the screen corner using
//   screenMarginX/Y. Set anchorTopRight false to use screenOffsetX/Y from
//   the top-left of the view instead.
// ---------------------------------------------------------------------------

struct MinimapConfig
{
	bool enabled = true;

	// Screen placement (HUD coords = camera + offset, fixed on screen)
	bool anchorTopRight = true;
	float screenMarginX = 14.0f;
	float screenMarginY = 14.0f;
	float screenOffsetX = 14.0f;
	float screenOffsetY = 14.0f;

	float panelWidth = 220.0f;
	float panelHeight = 165.0f;
	float innerPadding = 6.0f;
	float borderThickness = 2.0f;

	// Enemies farther than this from the player are hidden on the minimap.
	float enemyRevealRadius = 520.0f;

	bool drawWalls = true;
	float wallLineWidth = 1.25f;
	float wallR = 0.55f;
	float wallG = 0.55f;
	float wallB = 0.60f;
	float wallA = 0.90f;

	float backgroundR = 0.04f;
	float backgroundG = 0.05f;
	float backgroundB = 0.07f;
	float backgroundA = 0.82f;

	float borderR = 0.25f;
	float borderG = 0.70f;
	float borderB = 0.35f;
	float borderA = 0.95f;

	float playerDotRadius = 4.0f;
	float playerR = 0.35f;
	float playerG = 0.85f;
	float playerB = 1.0f;
	float playerA = 1.0f;

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
