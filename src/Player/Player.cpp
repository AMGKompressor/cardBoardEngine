#include "Player.h"

#include "Collision2D.h"
#include "Map/Map.h"

#include "renderer.h"
#include "sprite.h"

#include <SDL.h>

#include <algorithm>
#include <cmath>

namespace CardBoard
{
	namespace
	{
		void drawWorldCircleOutline(
			Renderer& renderer,
			float cx,
			float cy,
			float radius,
			int segments,
			float cr,
			float cg,
			float cb,
			float ca)
		{
			if (segments < 3 || segments > 64)
			{
				return;
			}
			float xy[128];
			const float twoPi = 6.2831853f;
			for (int i = 0; i < segments; ++i)
			{
				const float ang = twoPi * static_cast<float>(i) / static_cast<float>(segments);
				xy[i * 2 + 0] = cx + std::cos(ang) * radius;
				xy[i * 2 + 1] = cy + std::sin(ang) * radius;
			}
			renderer.drawWorldLineLoop(xy, segments, cr, cg, cb, ca);
		}
	}

	bool Player::initialize(Renderer& renderer, const PlayerConfig& config, float spawnX, float spawnY)
	{
		shutdown();
		mConfig = config;
		mX = spawnX;
		mY = spawnY;
		mFacingDeg = 0.0f;
		mMoveSpeed = mConfig.walkSpeed;
		mHitboxHalfW = mConfig.hitboxHalfW;
		mHitboxHalfH = mConfig.hitboxHalfH;
		mFlashlightOn = false;
		mFlashlightChargeSeconds = mConfig.flashlightMeter.maxChargeSeconds;

		mSprite = renderer.createSprite("textures/board8x8.png");
		if (mSprite == nullptr)
		{
			return false;
		}

		const float sunekuScale =
			mConfig.desiredSpriteBoxSize / static_cast<float>(mSprite->getWidth());
		mSprite->setScale(sunekuScale);
		mSprite->setRedTint(mConfig.bodyTintR);
		mSprite->setGreenTint(mConfig.bodyTintG);
		mSprite->setBlueTint(mConfig.bodyTintB);
		mHitboxHalfW = static_cast<float>(mSprite->getWidth()) * 0.5f;
		mHitboxHalfH = static_cast<float>(mSprite->getHeight()) * 0.5f;

		mHitboxDebugSprite = renderer.createSprite("textures/board8x8.png");
		if (mHitboxDebugSprite != nullptr)
		{
			mHitboxDebugSprite->setScale(sunekuScale);
			mHitboxDebugSprite->setRedTint(1.0f);
			mHitboxDebugSprite->setGreenTint(0.0f);
			mHitboxDebugSprite->setBlueTint(0.0f);
			mHitboxDebugSprite->setAlpha(0.3f);
		}

		mSprite->setX(static_cast<int>(mX));
		mSprite->setY(static_cast<int>(mY));
		if (mHitboxDebugSprite != nullptr)
		{
			mHitboxDebugSprite->setX(static_cast<int>(mX));
			mHitboxDebugSprite->setY(static_cast<int>(mY));
		}
		return true;
	}

	void Player::shutdown()
	{
		delete mHitboxDebugSprite;
		mHitboxDebugSprite = nullptr;
		delete mSprite;
		mSprite = nullptr;
		mWalkNoisePulses.clear();
		mSprintNoisePulses.clear();
	}

	void Player::toggleFlashlight()
	{
		if (mFlashlightOn)
		{
			mFlashlightOn = false;
			return;
		}

		if (mFlashlightChargeSeconds >= mConfig.flashlightMeter.minChargeToToggleOn)
		{
			mFlashlightOn = true;
		}
	}

	float Player::flashlightChargeRatio() const
	{
		const float maxCharge = mConfig.flashlightMeter.maxChargeSeconds;
		if (maxCharge <= 0.0f)
		{
			return 0.0f;
		}
		return std::min(1.0f, std::max(0.0f, mFlashlightChargeSeconds / maxCharge));
	}

	void Player::emitNoisePulse(float x, float y, bool loud)
	{
		NoisePulse pulse;
		pulse.age = 0.0f;
		pulse.cx = x;
		pulse.cy = y;
		pulse.maxRadius =
			loud ? mConfig.footstep.sprintNoiseRadius : mConfig.footstep.walkNoiseRadius;
		if (loud)
		{
			mSprintNoisePulses.push_back(pulse);
		}
		else
		{
			mWalkNoisePulses.push_back(pulse);
		}
	}

	void Player::advanceNoisePulses(float deltaTime)
	{
		const float duration = mConfig.footstep.pulseDuration;
		for (NoisePulse& pulse : mWalkNoisePulses)
		{
			pulse.age += deltaTime;
		}
		mWalkNoisePulses.erase(
			std::remove_if(
				mWalkNoisePulses.begin(),
				mWalkNoisePulses.end(),
				[duration](const NoisePulse& p) { return p.age >= duration; }),
			mWalkNoisePulses.end());

		for (NoisePulse& pulse : mSprintNoisePulses)
		{
			pulse.age += deltaTime;
		}
		mSprintNoisePulses.erase(
			std::remove_if(
				mSprintNoisePulses.begin(),
				mSprintNoisePulses.end(),
				[duration](const NoisePulse& p) { return p.age >= duration; }),
			mSprintNoisePulses.end());
	}

	void Player::updateFacingTowardMouse(float deltaTime, float cameraX, float cameraY)
	{
		int mouseX = 0;
		int mouseY = 0;
		SDL_GetMouseState(&mouseX, &mouseY);

		const float worldMouseX = cameraX + static_cast<float>(mouseX);
		const float worldMouseY = cameraY + static_cast<float>(mouseY);

		const float dx = worldMouseX - mX;
		const float dy = worldMouseY - mY;
		const float lenSq = dx * dx + dy * dy;
		if (lenSq < 4.0f)
		{
			return;
		}

		const float targetDeg =
			-std::atan2(dy, dx) * 57.2957795f + mConfig.spriteFacingOffsetDeg;

		const float maxStep = mConfig.turnRateDegPerSec * deltaTime;
		const float delta = shortestAngleDeltaDegrees(mFacingDeg, targetDeg);

		if (std::fabs(delta) <= maxStep || std::fabs(delta) <= mConfig.aimSnapDegrees)
		{
			mFacingDeg = targetDeg;
		}
		else
		{
			mFacingDeg += (delta >= 0.0f) ? maxStep : -maxStep;
		}

		mFacingDeg = wrap360(mFacingDeg);
		if (mSprite != nullptr)
		{
			mSprite->setAngle(mFacingDeg);
		}
		if (mHitboxDebugSprite != nullptr)
		{
			mHitboxDebugSprite->setAngle(mFacingDeg);
		}
	}

	void Player::update(
		float deltaTime,
		const Map& map,
		float cameraX,
		float cameraY,
		bool w,
		bool a,
		bool s,
		bool d,
		bool sprintHeld,
		bool stunHeld)
	{
		(void)cameraX;
		(void)cameraY;

		updateFacingTowardMouse(deltaTime, cameraX, cameraY);

		float dx = 0.0f;
		float dy = 0.0f;
		if (w) { dy -= 1.0f; }
		if (s) { dy += 1.0f; }
		if (a) { dx -= 1.0f; }
		if (d) { dx += 1.0f; }

		if (dx != 0.0f && dy != 0.0f)
		{
			const float invLen = 0.70710678f;
			dx *= invLen;
			dy *= invLen;
		}

		const bool moveInput = (dx != 0.0f || dy != 0.0f);
		const float targetSpeed =
			(moveInput && sprintHeld) ? mConfig.sprintSpeed : mConfig.walkSpeed;

		if (mMoveSpeed < targetSpeed)
		{
			mMoveSpeed = std::min(
				targetSpeed,
				mMoveSpeed + mConfig.speedRampUpPerSec * deltaTime);
		}
		else if (mMoveSpeed > targetSpeed)
		{
			mMoveSpeed = std::max(
				targetSpeed,
				mMoveSpeed - mConfig.speedRampDownPerSec * deltaTime);
		}

		const float nextX = mX + dx * mMoveSpeed * deltaTime;
		const float nextY = mY + dy * mMoveSpeed * deltaTime;

		float worldHalfX = 0.0f;
		float worldHalfY = 0.0f;
		orientedRectWorldAabbHalfExtents(
			mHitboxHalfW, mHitboxHalfH, mFacingDeg, worldHalfX, worldHalfY);

		float clampedX = nextX;
		float clampedY = nextY;
		map.constrainPlayerCenter(
			clampedX,
			clampedY,
			worldHalfX,
			worldHalfY,
			mHitboxHalfW,
			mHitboxHalfH,
			mFacingDeg);

		if (map.pollNewWallContact(clampedX, clampedY, mHitboxHalfW, mHitboxHalfH, mFacingDeg))
		{
			emitNoisePulse(clampedX, clampedY, sprintHeld);
		}

		advanceNoisePulses(deltaTime);

		if (moveInput)
		{
			if (mWalkPulseCooldown > 0.0f)
			{
				mWalkPulseCooldown = std::max(0.0f, mWalkPulseCooldown - deltaTime);
			}
			if (mSprintPulseCooldown > 0.0f)
			{
				mSprintPulseCooldown = std::max(0.0f, mSprintPulseCooldown - deltaTime);
			}

			const float stepInterval =
				sprintHeld ? mConfig.footstep.intervalSprint : mConfig.footstep.intervalWalk;
			mFootstepCooldown -= deltaTime;
			if (mFootstepCooldown <= 0.0f)
			{
				if (sprintHeld)
				{
					if (mSprintPulseCooldown <= 0.0f)
					{
						emitNoisePulse(mX, mY, true);
						mSprintPulseCooldown = mConfig.footstep.sprintPulseInterval;
					}
				}
				else
				{
					if (mWalkPulseCooldown <= 0.0f)
					{
						emitNoisePulse(mX, mY, false);
						mWalkPulseCooldown = mConfig.footstep.walkPulseInterval;
					}
					mSprintPulseCooldown = 0.0f;
				}
				mFootstepCooldown = stepInterval;
			}
		}
		else
		{
			mFootstepCooldown = 0.0f;
			mWalkPulseCooldown = 0.0f;
			mSprintPulseCooldown = 0.0f;
		}

		mX = clampedX;
		mY = clampedY;

		if (mSprite != nullptr)
		{
			mSprite->setX(static_cast<int>(mX));
			mSprite->setY(static_cast<int>(mY));
		}
		if (mHitboxDebugSprite != nullptr)
		{
			mHitboxDebugSprite->setX(static_cast<int>(mX));
			mHitboxDebugSprite->setY(static_cast<int>(mY));
		}

		mFlashlightStunActive = (mFlashlightOn && stunHeld);

		const FlashlightMeterConfig& meter = mConfig.flashlightMeter;
		const FlashlightStunConfig& stun = mConfig.flashlightStun;
		const float drainPerSecond = meter.drainPerSecond
			+ (mFlashlightStunActive ? stun.extraDrainPerSecond : 0.0f);
		if (mFlashlightOn)
		{
			mFlashlightChargeSeconds = std::max(
				0.0f,
				mFlashlightChargeSeconds - drainPerSecond * deltaTime);
			if (mFlashlightChargeSeconds <= 0.0f)
			{
				mFlashlightOn = false;
				mFlashlightStunActive = false;
			}
		}
		else
		{
			mFlashlightChargeSeconds = std::min(
				meter.maxChargeSeconds,
				mFlashlightChargeSeconds + meter.rechargePerSecond * deltaTime);
		}
	}

	void Player::drawSprite(Renderer& renderer) const
	{
		if (mSprite != nullptr)
		{
			mSprite->draw(renderer);
		}
	}

	void Player::drawHitboxDebug(Renderer& renderer) const
	{
		if (mShowHitboxDebug && mHitboxDebugSprite != nullptr)
		{
			mHitboxDebugSprite->draw(renderer);
		}
	}

	void Player::drawFlashlightMask(Renderer& renderer, const Map& map, float cameraX, float cameraY) const
	{
		if (mShowHitboxDebug)
		{
			return;
		}

		const FlashlightConfig& fl = mConfig.flashlight;
		const FlashlightStunConfig& stun = mConfig.flashlightStun;
		const bool stunVisual = mFlashlightOn && mFlashlightStunActive;
		const float halfAngle = stunVisual ? fl.halfAngleDeg * stun.beamHalfAngleMultiplier : fl.halfAngleDeg;
		const float beamRange = stunVisual ? fl.beamRange * stun.beamRangeMultiplier : fl.beamRange;
		const float featherDeg = stunVisual
			? std::max(0.5f, fl.featherDeg * stun.coneFeatherMultiplier)
			: fl.featherDeg;
		renderer.drawFlashlightMask(
			mX,
			mY,
			mFacingDeg,
			mFlashlightOn,
			halfAngle,
			beamRange,
			featherDeg,
			fl.ambientRadius,
			fl.ambientFeather,
			fl.maskAlpha,
			cameraX,
			cameraY,
			map.wireFlat(),
			map.segmentCount());
	}

	FlashlightStunQuery Player::flashlightStunQuery() const
	{
		FlashlightStunQuery query;
		query.active = mFlashlightOn && mFlashlightStunActive;
		query.originX = mX;
		query.originY = mY;
		query.facingDeg = mFacingDeg;
		query.halfAngleDeg = mConfig.flashlight.halfAngleDeg
			* mConfig.flashlightStun.beamHalfAngleMultiplier;
		query.range = mConfig.flashlight.beamRange
			* mConfig.flashlightStun.beamRangeMultiplier;
		return query;
	}

	void Player::drawFlashlightMeter(Renderer& renderer, float cameraX, float cameraY) const
	{
		const FlashlightMeterConfig& meter = mConfig.flashlightMeter;
		if (meter.width <= 2.0f || meter.height <= 2.0f)
		{
			return;
		}

		const float left = cameraX + meter.screenOffsetX;
		const float top = cameraY + meter.screenOffsetY;
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

		const float ratio = flashlightChargeRatio();
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

	void Player::drawNoisePulses(Renderer& renderer) const
	{
		if (!mShowHitboxDebug)
		{
			return;
		}

		const float duration = mConfig.footstep.pulseDuration;
		for (const NoisePulse& pulse : mWalkNoisePulses)
		{
			if (pulse.maxRadius <= 1.0f)
			{
				continue;
			}
			const float t = std::min(1.0f, pulse.age / duration);
			const float radius = pulse.maxRadius * t;
			const float alpha = 0.55f * (1.0f - t);
			drawWorldCircleOutline(
				renderer,
				pulse.cx,
				pulse.cy,
				radius,
				36,
				0.35f,
				0.85f,
				1.0f,
				alpha);
		}
		for (const NoisePulse& pulse : mSprintNoisePulses)
		{
			if (pulse.maxRadius <= 1.0f)
			{
				continue;
			}
			const float t = std::min(1.0f, pulse.age / duration);
			const float radius = pulse.maxRadius * t;
			const float alpha = 0.7f * (1.0f - t);
			drawWorldCircleOutline(
				renderer,
				pulse.cx,
				pulse.cy,
				radius,
				36,
				1.0f,
				0.45f,
				0.2f,
				alpha);
		}
	}
}
