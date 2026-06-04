#pragma once

// =============================================================================
// MinimapConfig — edit here, then rebuild. Live sliders: ` ImGui → Minimap.
//
// HOW TO FINE-TUNE ZOOM (centerOnPlayer = true)
// ----------------------------------------------
// viewRadiusWorld = how many world pixels you see from the player to the inner
//   edge of the minimap. This is the main zoom knob.
//
//   Smaller  → more zoomed in  (e.g. 250 = tight corridors, close radar)
//   Larger   → more zoomed out (e.g. 600 = see more of the level at once)
//
//   Typical range: 200–700. Default 400 is a medium-local view.
//
// panelWidth / panelHeight — bigger panel = same zoom but easier to read;
//   does not change how much world fits (that is viewRadiusWorld).
//
// innerPadding — gap between green border and map; walls are clipped inside.
//
// enemyRevealRadius — enemy dots only if this close to you (world px).
//   Match viewRadiusWorld so blips match what you see, or lower to hide far foes.
//
// centerOnPlayer = false — ignores viewRadiusWorld; shows the entire map fitted
//   inside the panel (old overview mode).
// =============================================================================

struct MinimapConfig
{
	bool enabled = true;

	// --- Zoom / follow (finetune viewRadiusWorld first) ---
	bool centerOnPlayer = true;
	float viewRadiusWorld = 400.0f;

	// --- Panel on screen (HUD = camera + offset) ---
	bool anchorTopRight = true;
	float screenMarginX = 14.0f;
	float screenMarginY = 14.0f;
	float screenOffsetX = 14.0f;
	float screenOffsetY = 14.0f;

	float panelWidth = 220.0f;
	float panelHeight = 165.0f;
	float innerPadding = 6.0f;
	float borderThickness = 2.0f;

	float enemyRevealRadius = 400.0f;

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
