#include "UI.h"
#include "Player_1.h"
#include "PlayerConfig_1.h"

#include "renderer.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>

UI::UI()
{

}

UI::~UI()
{
	mConfig = NULL;
	ui_player = NULL;
}

bool UI::initialise(Renderer& renderer, Player* player, PlayerConfig* config)
{
	mConfig = config;
	ui_player = player;

	return true;
}

void UI::process(float deltaTime)
{

}

void UI::draw(Renderer& renderer, float cameraX, float cameraY)
{
	if (ui_player->getDarkStatus() || ui_player->getSanityPercentage() <= 9.0f)
	{
		drawSanityMeter(renderer, cameraX, cameraY);
	}

	if (ui_player->isRunning == true || ui_player->staminaPercentage <= 85.0f)
	{
		drawStaminaMeter(renderer, cameraX, cameraY);
	}

	drawHealthMeter(renderer, cameraX, cameraY);
}

void UI::drawStaminaMeter(Renderer& renderer, float cameraX, float cameraY)
{
	const StaminaMeterConfig& meter = mConfig->staminaMeter;
	if (meter.width <= 2.0f || meter.height <= 2.0f)
	{
		return;
	}

	const float left = ui_player->mX - (meter.width * 0.5f);
	const float top = ui_player->mY - 90.0f;

	const float centerX = left + meter.width * 0.5f;
	const float centerY = top + meter.height * 0.5f;

	renderer.drawWorldAxisAlignedQuad(
		centerX,
		centerY,
		meter.width * 0.5f,
		meter.height * 0.5f,
		0.05f,
		0.05f,
		0.05f,
		0.85f);

	const float ratio = staminaRatio();
	const float innerHeight = std::max(1.0f, meter.height - meter.border * 2.0f);
	const float innerWidthMax = std::max(1.0f, meter.width - meter.border * 2.0f);
	const float innerWidth = std::max(1.0f, innerWidthMax * ratio);
	const float innerLeft = left + meter.border;
	const float innerCenterX = innerLeft + innerWidth * 0.5f;
	const float innerCenterY = top + meter.height * 0.5f;

	const float red = 0.90f;
	const float green = 0.90f;
	const float blue = 0.10f;

	renderer.drawWorldAxisAlignedQuad(
		innerCenterX,
		innerCenterY,
		innerWidth * 0.5f,
		innerHeight * 0.5f,
		red,
		green,
		blue,
		0.95f);
}

void UI::adjustStamina(float deltaTime)
{
	if (ui_player->isRunning == true)
	{
		ui_player->staminaPercentage = std::max(
			0.0f,
			ui_player->staminaPercentage -
			mConfig->staminaMeter.drainPerSecond * deltaTime);
	}
	else
	{
		ui_player->staminaPercentage = std::min(
			mConfig->staminaMeter.maxStamina,
			ui_player->staminaPercentage +
			mConfig->staminaMeter.rechargePerSecond * deltaTime);
	}
}

float UI::staminaRatio()
{
	const float maxStamina = mConfig->staminaMeter.maxStamina;
	if (maxStamina <= 0.0f)
	{
		return 0.0f;
	}


	return std::min(1.0f, std::max(0.0f, ui_player->staminaPercentage / maxStamina));
}

void UI::drawSanityMeter(Renderer& renderer, float cameraX, float cameraY)
{
	const SanityMeterConfig& meter = mConfig->sanityMeter;
	if (meter.width <= 2.0f || meter.height <= 2.0f)
	{
		return;
	}

	const float left = ui_player->mX - (meter.width * 0.5f);
	const float top = ui_player->mY - 80.0f;

	const float centerX = left + meter.width * 0.5f;
	const float centerY = top + meter.height * 0.5f;

	renderer.drawWorldAxisAlignedQuad(
		centerX,
		centerY,
		meter.width * 0.5f,
		meter.height * 0.5f,
		0.05f,
		0.05f,
		0.05f,
		0.85f);

	const float ratio = sanityRatio();
	const float innerHeight = std::max(1.0f, meter.height - meter.border * 2.0f);
	const float innerWidthMax = std::max(1.0f, meter.width - meter.border * 2.0f);
	const float innerWidth = std::max(1.0f, innerWidthMax * ratio);
	const float innerLeft = left + meter.border;
	const float innerCenterX = innerLeft + innerWidth * 0.5f;
	const float innerCenterY = top + meter.height * 0.5f;

	const float red = (ratio < 0.35f) ? 0.95f : 0.20f;
	const float green = (ratio < 0.35f) ? 0.25f : 0.90f;
	const float blue = 0.20f;

	renderer.drawWorldAxisAlignedQuad(
		innerCenterX,
		innerCenterY,
		innerWidth * 0.5f,
		innerHeight * 0.5f,
		red,
		green,
		blue,
		0.95f);
}
bool UI::noSanity(float deltaTime)
{
	if (ui_player->sanityPercentage <= 0.1f)
	{
		adjustHealth(deltaTime);
		return true;
	}
	return false;
}

void UI::adjustSanity(float deltaTime)
{
	if (ui_player->inDark)
	{
		ui_player->sanityPercentage = std::max(
			0.0f,
			ui_player->sanityPercentage -
			mConfig->sanityMeter.drainPerSecond * deltaTime);
	}
	else
	{
		ui_player->sanityPercentage = std::min(
			mConfig->sanityMeter.maxCharge,
			ui_player->sanityPercentage +
			mConfig->sanityMeter.rechargePerSecond * deltaTime);
	}

	noSanity(deltaTime);
}

float UI::sanityRatio()
{
	const float maxCharge = mConfig->sanityMeter.maxCharge;
	if (maxCharge <= 0.0f)
	{
		return 0.0f;
	}


	return std::min(1.0f, std::max(0.0f, ui_player->sanityPercentage / maxCharge));

}

void UI::drawHealthMeter(Renderer& renderer, float cameraX, float cameraY)
{
	const HealthMeterConfig& meter = mConfig->healthMeter;
	if (meter.width <= 2.0f || meter.height <= 2.0f)
	{
		return;
	}

	const float left = ui_player->mX - (meter.width * 0.5f);
	const float top = ui_player->mY - 100.0f;

	const float centerX = left + meter.width * 0.5f;
	const float centerY = top + meter.height * 0.5f;

	renderer.drawWorldAxisAlignedQuad(
		centerX,
		centerY,
		meter.width * 0.5f,
		meter.height * 0.5f,
		0.05f,
		0.05f,
		0.05f,
		0.85f);

	const float ratio = healthRatio();
	const float innerHeight = std::max(1.0f, meter.height - meter.border * 2.0f);
	const float innerWidthMax = std::max(1.0f, meter.width - meter.border * 2.0f);
	const float innerWidth = std::max(1.0f, innerWidthMax * ratio);
	const float innerLeft = left + meter.border;
	const float innerCenterX = innerLeft + innerWidth * 0.5f;
	const float innerCenterY = top + meter.height * 0.5f;

	const float red = (ratio < 0.35f) ? 0.95f : 0.20f;
	const float green = (ratio < 0.35f) ? 0.25f : 0.90f;
	const float blue = 0.20f;

	renderer.drawWorldAxisAlignedQuad(
		innerCenterX,
		innerCenterY,
		innerWidth * 0.5f,
		innerHeight * 0.5f,
		red,
		green,
		blue,
		0.95f);
}

void UI::adjustHealth(float deltaTime)
{
	ui_player->playerHealth = std::max(
		0.0f,
		ui_player->playerHealth -
		mConfig->healthMeter.decay * deltaTime);
}

float UI::healthRatio()
{
	const float maxCharge = mConfig->healthMeter.maxCharge;
	if (maxCharge <= 0.0f)
	{
		return 0.0f;
	}

	return std::min(1.0f, std::max(0.0f, ui_player->playerHealth / maxCharge));
}