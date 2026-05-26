#include "Player.h"

#include "Collision2D.h"
#include "Map.h"

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
		mFlashlightOn = !mFlashlightOn;
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
		bool sprintHeld)
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
		renderer.drawFlashlightMask(
			mX,
			mY,
			mFacingDeg,
			mFlashlightOn,
			fl.halfAngleDeg,
			fl.beamRange,
			fl.featherDeg,
			fl.ambientRadius,
			fl.ambientFeather,
			fl.maskAlpha,
			cameraX,
			cameraY,
			map.wireFlat(),
			map.segmentCount());
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
