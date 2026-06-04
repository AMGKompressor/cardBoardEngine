// cardBoard -- enemy AI: noise-seeking pathfinding, stun reaction, contact damage
#pragma once

#include "EnemyConfig.h"

#include <vector>

class Renderer;
class Sprite;

namespace CardBoard
{
	class Map;
	class Player;
	struct NoisePulse;
	struct FlashlightStunQuery;

	enum class EnemyState
	{
		Roaming,
		Chasing,
		Stunned,
		Dead
	};

	class Enemy
	{
	public:
		bool initialize(
			Renderer& renderer,
			const EnemyConfig& config,
			float spawnX,
			float spawnY);
		void shutdown();

		void update(
			float deltaTime,
			const Map& map,
			Player& player);

		void drawSprite(Renderer& renderer) const;
		void drawDebug(Renderer& renderer) const;

		float x()      const { return mX; }
		float y()      const { return mY; }
		bool  isDead() const { return mState == EnemyState::Dead; }

		void setShowDebug(bool show) { mShowDebug = show; }
		bool showDebug()       const { return mShowDebug; }

	private:
		void updateRoaming(float deltaTime, const Map& map, const Player& player);
		void updateChasing(float deltaTime, const Map& map, const Player& player);
		void updateStunned(float deltaTime);

		void applyMovement(float dx, float dy, float speed, float deltaTime, const Map& map);
		void faceDirection(float dx, float dy, float deltaTime);

		bool canHearPlayer(const Player& player) const;
		bool canSeePlayer(const Player& player)  const;
		void tryApplyStun(const Player& player);
		void tryDealContactDamage(float deltaTime, Player& player);

		EnemyConfig mConfig;
		Sprite* mSprite = nullptr;

		float mX = 0.0f;
		float mY = 0.0f;
		float mFacingDeg = 0.0f;
		float mHealth = 0.0f;

		EnemyState mState = EnemyState::Roaming;
		float      mStunTimer = 0.0f;
		float      mStunCooldownTimer = 0.0f;

		float mRoamTargetX = 0.0f;
		float mRoamTargetY = 0.0f;
		bool  mHasRoamTarget = false;

		bool  mShowDebug = false;
	};
}