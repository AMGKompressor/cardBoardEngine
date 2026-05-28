// cardBoard — player movement, hitbox, flashlight, and footstep noise pulses
#pragma once

#include "PlayerConfig.h"

#include <vector>

class Renderer;
class Sprite;

namespace CardBoard
{
	class Map;

	struct NoisePulse
	{
		float age = 0.0f;
		float maxRadius = 0.0f;
		float cx = 0.0f;
		float cy = 0.0f;
	};

	struct FlashlightStunQuery
	{
		bool active = false;
		float originX = 0.0f;
		float originY = 0.0f;
		float facingDeg = 0.0f;
		float halfAngleDeg = 0.0f;
		float range = 0.0f;
	};

	class Player
	{
	public:
		bool initialize(Renderer& renderer, const PlayerConfig& config, float spawnX, float spawnY);
		void shutdown();

		void update(
			float deltaTime,
			const Map& map,
			float cameraX,
			float cameraY,
			bool w,
			bool a,
			bool s,
			bool d,
			bool sprintHeld,
			bool stunHeld);

		void updateFacingTowardMouse(float deltaTime, float cameraX, float cameraY);
		void toggleFlashlight();

		void drawSprite(Renderer& renderer) const;
		void drawHitboxDebug(Renderer& renderer) const;
		void drawFlashlightMask(Renderer& renderer, const Map& map, float cameraX, float cameraY) const;
		void drawFlashlightMeter(Renderer& renderer, float cameraX, float cameraY) const;
		void drawNoisePulses(Renderer& renderer) const;

		float x() const { return mX; }
		float y() const { return mY; }
		float facingDeg() const { return mFacingDeg; }
		float moveSpeed() const { return mMoveSpeed; }
		bool flashlightOn() const { return mFlashlightOn; }
		float flashlightChargeSeconds() const { return mFlashlightChargeSeconds; }
		float flashlightChargeRatio() const;
		bool flashlightStunActive() const { return mFlashlightStunActive; }
		FlashlightStunQuery flashlightStunQuery() const;
		bool showHitboxDebug() const { return mShowHitboxDebug; }
		void setShowHitboxDebug(bool show) { mShowHitboxDebug = show; }

		const std::vector<NoisePulse>& walkNoisePulses() const { return mWalkNoisePulses; }
		const std::vector<NoisePulse>& sprintNoisePulses() const { return mSprintNoisePulses; }

	private:
		void emitNoisePulse(float x, float y, bool loud);
		void advanceNoisePulses(float deltaTime);

		PlayerConfig mConfig;
		Sprite* mSprite = nullptr;
		Sprite* mHitboxDebugSprite = nullptr;

		float mX = 0.0f;
		float mY = 0.0f;
		float mFacingDeg = 0.0f;
		float mMoveSpeed = 0.0f;
		float mHitboxHalfW = 48.0f;
		float mHitboxHalfH = 48.0f;
		float mFlashlightChargeSeconds = 0.0f;

		bool mFlashlightOn = false;
		bool mFlashlightStunActive = false;
		bool mShowHitboxDebug = false;

		float mFootstepCooldown = 0.0f;
		float mWalkPulseCooldown = 0.0f;
		float mSprintPulseCooldown = 0.0f;

		std::vector<NoisePulse> mWalkNoisePulses;
		std::vector<NoisePulse> mSprintNoisePulses;
	};
}
