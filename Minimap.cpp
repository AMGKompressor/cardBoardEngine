// Draws a screen-fixed panel: map walls (same data as collision), player dot,
// enemy dots only within MinimapConfig::enemyRevealRadius of the player.

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
		float mapOriginX = 0.0f;
		float mapOriginY = 0.0f;
		float scale = 1.0f;
	};

	float distSq(float ax, float ay, float bx, float by)
	{
		const float dx = bx - ax;
		const float dy = by - ay;
		return dx * dx + dy * dy;
	}

	PanelLayout buildLayout(
		const MinimapConfig& config,
		float mapWidth,
		float mapHeight,
		float cameraX,
		float cameraY,
		float viewWidth,
		float viewHeight)
	{
		PanelLayout layout;

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
		if (mapWidth > 1.0f && mapHeight > 1.0f)
		{
			layout.scale = std::min(innerW / mapWidth, innerH / mapHeight);
		}

		const float usedW = mapWidth * layout.scale;
		const float usedH = mapHeight * layout.scale;
		layout.mapOriginX =
			layout.panelLeft + config.innerPadding + (innerW - usedW) * 0.5f;
		layout.mapOriginY =
			layout.panelTop + config.innerPadding + (innerH - usedH) * 0.5f;

		return layout;
	}

	void worldToPanel(
		float worldX,
		float worldY,
		const PanelLayout& layout,
		float& outX,
		float& outY)
	{
		outX = layout.mapOriginX + worldX * layout.scale;
		outY = layout.mapOriginY + worldY * layout.scale;
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

	const float mapW = map.width();
	const float mapH = map.height();
	const PanelLayout layout = buildLayout(
		mConfig,
		mapW,
		mapH,
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

			for (int s = 0; s < segmentCount; ++s)
			{
				const int o = s * 4;
				float x0 = 0.0f;
				float y0 = 0.0f;
				float x1 = 0.0f;
				float y1 = 0.0f;
				worldToPanel(wire[o + 0], wire[o + 1], layout, x0, y0);
				worldToPanel(wire[o + 2], wire[o + 3], layout, x1, y1);
				lineVerts.push_back(x0);
				lineVerts.push_back(y0);
				lineVerts.push_back(x1);
				lineVerts.push_back(y1);
			}

			renderer.drawWorldLineSegments(
				lineVerts.data(),
				segmentCount,
				mConfig.wallR,
				mConfig.wallG,
				mConfig.wallB,
				mConfig.wallA);
		}
	}

	const float revealSq =
		mConfig.enemyRevealRadius * mConfig.enemyRevealRadius;
	const float playerX = player.x();
	const float playerY = player.y();

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

	float playerDotX = 0.0f;
	float playerDotY = 0.0f;
	worldToPanel(playerX, playerY, layout, playerDotX, playerDotY);
	drawDot(
		renderer,
		playerDotX,
		playerDotY,
		mConfig.playerDotRadius,
		mConfig.playerR,
		mConfig.playerG,
		mConfig.playerB,
		mConfig.playerA);

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
		mConfig.borderA,
		mConfig.borderThickness);
}

void Minimap::debugDraw()
{
	ImGui::Text("Minimap");
	ImGui::Checkbox("enabled", &mConfig.enabled);
	ImGui::SliderFloat("enemy reveal radius", &mConfig.enemyRevealRadius, 80.0f, 1200.0f);
	ImGui::Checkbox("draw walls", &mConfig.drawWalls);
}
