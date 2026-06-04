// =============================================================================
// Minimap.cpp — corner HUD map (zoomed radar or full-level overview).
//
// ZOOMED MODE (centerOnPlayer = true):
//   Player at panel center; walls/enemies use offset from player * scaleX/scaleY.
//   Zoom finetune: MinimapConfig::viewRadiusWorld (lower = zoom in, higher = out).
//   ImGui ` window → "view radius (zoom)" slider tests values without rebuild.
//
// DRAW ORDER: background → walls → enemies → player dot → border
// =============================================================================

#include "Minimap.h"

#include "Enemy.h"
#include "EnemyManager.h"
#include "EnemyType.h"
#include "Map_1.h"
#include "Player_1.h"
#include "renderer.h"

#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace
{
	struct PanelLayout
	{
		float panelLeft = 0.0f;
		float panelTop = 0.0f;
		float viewCenterX = 0.0f;
		float viewCenterY = 0.0f;
		float mapOriginX = 0.0f;
		float mapOriginY = 0.0f;
		float scaleX = 1.0f;
		float scaleY = 1.0f;
		float clipLeft = 0.0f;
		float clipTop = 0.0f;
		float clipRight = 0.0f;
		float clipBottom = 0.0f;
		float playerWorldX = 0.0f;
		float playerWorldY = 0.0f;
		bool centerOnPlayer = true;
		float viewRadiusWorld = 400.0f;
	};

	float distSq(float ax, float ay, float bx, float by)
	{
		const float dx = bx - ax;
		const float dy = by - ay;
		return dx * dx + dy * dy;
	}

	// Skip wall segments that cannot intersect the zoomed view (performance).
	bool segmentNearView(
		float x0,
		float y0,
		float x1,
		float y1,
		float playerX,
		float playerY,
		float viewRadius)
	{
		const float minX = std::min(x0, x1) - viewRadius;
		const float maxX = std::max(x0, x1) + viewRadius;
		const float minY = std::min(y0, y1) - viewRadius;
		const float maxY = std::max(y0, y1) + viewRadius;
		return playerX >= minX && playerX <= maxX && playerY >= minY && playerY <= maxY;
	}

	// Liang-Barsky: trim a HUD line segment to the inner panel (stays inside green border).
	bool clipSegmentToRect(
		float& x0,
		float& y0,
		float& x1,
		float& y1,
		float left,
		float top,
		float right,
		float bottom)
	{
		const float dx = x1 - x0;
		const float dy = y1 - y0;
		float t0 = 0.0f;
		float t1 = 1.0f;

		auto clipEdge = [&](float p, float q) -> bool {
			if (std::fabs(p) < 1e-6f)
			{
				return q >= 0.0f;
			}
			const float r = q / p;
			if (p < 0.0f)
			{
				if (r > t1)
				{
					return false;
				}
				if (r > t0)
				{
					t0 = r;
				}
			}
			else
			{
				if (r < t0)
				{
					return false;
				}
				if (r < t1)
				{
					t1 = r;
				}
			}
			return true;
		};

		if (!clipEdge(-dx, x0 - left))
		{
			return false;
		}
		if (!clipEdge(dx, right - x0))
		{
			return false;
		}
		if (!clipEdge(-dy, y0 - top))
		{
			return false;
		}
		if (!clipEdge(dy, bottom - y0))
		{
			return false;
		}

		const float nx0 = x0 + t0 * dx;
		const float ny0 = y0 + t0 * dy;
		const float nx1 = x0 + t1 * dx;
		const float ny1 = y0 + t1 * dy;
		x0 = nx0;
		y0 = ny0;
		x1 = nx1;
		y1 = ny1;
		return t1 > t0 + 1e-5f;
	}

	bool pointInsideClip(const PanelLayout& layout, float x, float y)
	{
		return x >= layout.clipLeft && x <= layout.clipRight
			&& y >= layout.clipTop && y <= layout.clipBottom;
	}

	PanelLayout buildLayout(
		const MinimapConfig& config,
		float mapWidth,
		float mapHeight,
		float playerX,
		float playerY,
		float cameraX,
		float cameraY,
		float viewWidth,
		float viewHeight)
	{
		PanelLayout layout;
		layout.centerOnPlayer = config.centerOnPlayer;
		layout.viewRadiusWorld = std::max(80.0f, config.viewRadiusWorld);
		layout.playerWorldX = playerX;
		layout.playerWorldY = playerY;

		if (config.anchorTopRight)
		{
			layout.panelLeft =
				cameraX + viewWidth - config.panelWidth - config.screenMarginX;
			layout.panelTop = cameraY + config.screenMarginY;
		}
		else
		{
			layout.panelLeft = cameraX + config.screenOffsetX;
			layout.panelTop = cameraY + config.screenOffsetY;
		}

		const float innerW = std::max(8.0f, config.panelWidth - config.innerPadding * 2.0f);
		const float innerH = std::max(8.0f, config.panelHeight - config.innerPadding * 2.0f);
		const float innerLeft = layout.panelLeft + config.innerPadding;
		const float innerTop = layout.panelTop + config.innerPadding;

		layout.viewCenterX = innerLeft + innerW * 0.5f;
		layout.viewCenterY = innerTop + innerH * 0.5f;
		layout.clipLeft = innerLeft;
		layout.clipTop = innerTop;
		layout.clipRight = innerLeft + innerW;
		layout.clipBottom = innerTop + innerH;

		if (config.centerOnPlayer)
		{
			// viewRadiusWorld (world px) → inner panel edge; separate X/Y avoids border spill.
			layout.scaleX = (innerW * 0.5f) / layout.viewRadiusWorld;
			layout.scaleY = (innerH * 0.5f) / layout.viewRadiusWorld;
		}
		else if (mapWidth > 1.0f && mapHeight > 1.0f)
		{
			const float uniformScale = std::min(innerW / mapWidth, innerH / mapHeight);
			layout.scaleX = uniformScale;
			layout.scaleY = uniformScale;
			const float usedW = mapWidth * uniformScale;
			const float usedH = mapHeight * uniformScale;
			layout.mapOriginX = innerLeft + (innerW - usedW) * 0.5f;
			layout.mapOriginY = innerTop + (innerH - usedH) * 0.5f;
		}

		return layout;
	}

	void worldToPanel(
		float worldX,
		float worldY,
		const PanelLayout& layout,
		float& outX,
		float& outY)
	{
		if (layout.centerOnPlayer)
		{
			outX = layout.viewCenterX + (worldX - layout.playerWorldX) * layout.scaleX;
			outY = layout.viewCenterY + (worldY - layout.playerWorldY) * layout.scaleY;
		}
		else
		{
			outX = layout.mapOriginX + worldX * layout.scaleX;
			outY = layout.mapOriginY + worldY * layout.scaleY;
		}
	}

	void drawDot(
		Renderer& renderer,
		float centerX,
		float centerY,
		float radius,
		float r,
		float g,
		float b,
		float a)
	{
		renderer.drawWorldAxisAlignedQuad(
			centerX,
			centerY,
			radius,
			radius,
			r,
			g,
			b,
			a);
	}

	void enemyColor(EnemyType type, const MinimapConfig& config, float& r, float& g, float& b)
	{
		switch (type)
		{
		case EnemyType::Spirit:
			r = config.spiritR;
			g = config.spiritG;
			b = config.spiritB;
			break;
		case EnemyType::Hidden:
			r = config.hiddenR;
			g = config.hiddenG;
			b = config.hiddenB;
			break;
		case EnemyType::Revenant:
			r = config.revenantR;
			g = config.revenantG;
			b = config.revenantB;
			break;
		}
	}
}

void Minimap::draw(
	Renderer& renderer,
	float cameraX,
	float cameraY,
	float viewWidth,
	float viewHeight,
	const Map& map,
	const Player& player,
	const EnemyManager& enemies) const
{
	if (!mConfig.enabled || viewWidth < 32.0f || viewHeight < 32.0f)
	{
		return;
	}

	const float playerX = player.x();
	const float playerY = player.y();

	const PanelLayout layout = buildLayout(
		mConfig,
		map.width(),
		map.height(),
		playerX,
		playerY,
		cameraX,
		cameraY,
		viewWidth,
		viewHeight);

	const float panelCenterX = layout.panelLeft + mConfig.panelWidth * 0.5f;
	const float panelCenterY = layout.panelTop + mConfig.panelHeight * 0.5f;

	renderer.drawWorldAxisAlignedQuad(
		panelCenterX,
		panelCenterY,
		mConfig.panelWidth * 0.5f,
		mConfig.panelHeight * 0.5f,
		mConfig.backgroundR,
		mConfig.backgroundG,
		mConfig.backgroundB,
		mConfig.backgroundA);

	if (mConfig.drawWalls)
	{
		const float* wire = map.wireFlat();
		const int segmentCount = map.segmentCount();
		if (wire != nullptr && segmentCount > 0)
		{
			std::vector<float> lineVerts;
			lineVerts.reserve(static_cast<std::size_t>(segmentCount) * 4);

			const float wallCullRadius = layout.centerOnPlayer
				? layout.viewRadiusWorld * 1.15f
				: 1.0e9f;

			for (int s = 0; s < segmentCount; ++s)
			{
				const int o = s * 4;
				const float wx0 = wire[o + 0];
				const float wy0 = wire[o + 1];
				const float wx1 = wire[o + 2];
				const float wy1 = wire[o + 3];

				if (layout.centerOnPlayer
					&& !segmentNearView(wx0, wy0, wx1, wy1, playerX, playerY, wallCullRadius))
				{
					continue;
				}

				float x0 = 0.0f;
				float y0 = 0.0f;
				float x1 = 0.0f;
				float y1 = 0.0f;
				worldToPanel(wx0, wy0, layout, x0, y0);
				worldToPanel(wx1, wy1, layout, x1, y1);

				if (!clipSegmentToRect(
						x0,
						y0,
						x1,
						y1,
						layout.clipLeft,
						layout.clipTop,
						layout.clipRight,
						layout.clipBottom))
				{
					continue;
				}

				lineVerts.push_back(x0);
				lineVerts.push_back(y0);
				lineVerts.push_back(x1);
				lineVerts.push_back(y1);
			}

			if (!lineVerts.empty())
			{
				renderer.drawWorldLineSegments(
					lineVerts.data(),
					static_cast<int>(lineVerts.size() / 4),
					mConfig.wallR,
					mConfig.wallG,
					mConfig.wallB,
					mConfig.wallA);
			}
		}
	}

	const float revealSq =
		mConfig.enemyRevealRadius * mConfig.enemyRevealRadius;

	const int count = enemies.enemyCount();
	for (int i = 0; i < count; ++i)
	{
		const Enemy* enemy = enemies.enemyAt(i);
		if (enemy == nullptr || !enemy->active())
		{
			continue;
		}

		if (distSq(playerX, playerY, enemy->x(), enemy->y()) > revealSq)
		{
			continue;
		}

		float er = 1.0f;
		float eg = 1.0f;
		float eb = 1.0f;
		enemyColor(enemy->type(), mConfig, er, eg, eb);

		float dotX = 0.0f;
		float dotY = 0.0f;
		worldToPanel(enemy->x(), enemy->y(), layout, dotX, dotY);
		if (!pointInsideClip(layout, dotX, dotY))
		{
			continue;
		}
		drawDot(
			renderer,
			dotX,
			dotY,
			mConfig.enemyDotRadius,
			er,
			eg,
			eb,
			mConfig.enemyA);
	}

	// Player at panel center in zoom mode; mapped position in full-map mode.
	const float playerDotX = layout.centerOnPlayer ? layout.viewCenterX : 0.0f;
	const float playerDotY = layout.centerOnPlayer ? layout.viewCenterY : 0.0f;
	if (!layout.centerOnPlayer)
	{
		float px = 0.0f;
		float py = 0.0f;
		worldToPanel(playerX, playerY, layout, px, py);
		drawDot(
			renderer,
			px,
			py,
			mConfig.playerDotRadius,
			mConfig.playerR,
			mConfig.playerG,
			mConfig.playerB,
			mConfig.playerA);
	}
	else
	{
		drawDot(
			renderer,
			playerDotX,
			playerDotY,
			mConfig.playerDotRadius,
			mConfig.playerR,
			mConfig.playerG,
			mConfig.playerB,
			mConfig.playerA);
	}

	const float borderVerts[8] = {
		layout.panelLeft,
		layout.panelTop,
		layout.panelLeft + mConfig.panelWidth,
		layout.panelTop,
		layout.panelLeft + mConfig.panelWidth,
		layout.panelTop + mConfig.panelHeight,
		layout.panelLeft,
		layout.panelTop + mConfig.panelHeight
	};
	renderer.drawWorldLineLoop(
		borderVerts,
		4,
		mConfig.borderR,
		mConfig.borderG,
		mConfig.borderB,
		mConfig.borderA);
}

void Minimap::debugDraw()
{
	ImGui::Text("Minimap");
	ImGui::Checkbox("enabled", &mConfig.enabled);
	ImGui::Checkbox("center on player (zoom)", &mConfig.centerOnPlayer);
	if (mConfig.centerOnPlayer)
	{
		// Same as MinimapConfig::viewRadiusWorld — lower = zoom in, higher = zoom out.
		ImGui::SliderFloat("view radius (zoom)", &mConfig.viewRadiusWorld, 120.0f, 900.0f);
	}
	ImGui::SliderFloat("enemy reveal radius", &mConfig.enemyRevealRadius, 80.0f, 1200.0f);
	ImGui::Checkbox("draw walls", &mConfig.drawWalls);
}
