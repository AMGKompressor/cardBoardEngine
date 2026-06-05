#pragma once

#include "EnemyType.h"

#include <utility>
#include <vector>

class Renderer;
class NavGrid;
class Sprite;
class Map;
class Player;
class Item;

class Enemy
{
public:
	Enemy();
	~Enemy();

	bool initialize(EnemyType type, Renderer& renderer, const EnemyConfig& config, float spawnX, float spawnY);
	void shutdown();

	void update(
		float deltaTime,
		const Map& map,
		const Player& player,
		const std::vector<Item*>& worldItems,
		const EnemyConfig& config,
		const NavGrid* navGrid);

	void draw(Renderer& renderer) const;
	void drawPathDebug(Renderer& renderer) const;
	void debugDraw() const;

	EnemyType type() const { return mType; }
	bool active() const { return mActive; }
	float x() const { return mX; }
	float y() const { return mY; }

private:
	void updateSpirit(float deltaTime, const Map& map, const Player& player, const EnemyConfig& config, const NavGrid* navGrid);
	void updateHidden(
		float deltaTime,
		const Map& map,
		const Player& player,
		const std::vector<Item*>& worldItems,
		const EnemyConfig& config,
		const NavGrid* navGrid);
	void updateRevenant(float deltaTime, const Map& map, const Player& player, const EnemyConfig& config, const NavGrid* navGrid);

	void moveAlongPathTo(
		float deltaTime,
		const Map& map,
		const NavGrid& navGrid,
		float goalX,
		float goalY,
		float speed,
		float turnRateDegPerSec,
		bool faceMovement,
		const EnemyConfig& config);

	void clearPath();

	void moveWithCollision(
		float deltaTime,
		const Map& map,
		float dirX,
		float dirY,
		float speed,
		float turnRateDegPerSec,
		bool faceMovement);

	bool isStunned(float deltaTime, const EnemyConfig& config);
	bool isInFlashlightStun(const Player& player) const;
	void applyStun(float duration);
	void refreshAlertFromFootsteps(const Player& player, const EnemyConfig& config, float deltaTime);
	bool isWithinPlayerAwareness(const Player& player, const EnemyConfig& config) const;
	bool isWithinStunReactRange(const Player& player, const EnemyConfig& config) const;
	bool isAlerted() const { return mAlerted; }

	void tryTriggerStunReaction(
		const Player& player,
		const EnemyConfig& config,
		const NavGrid* navGrid);
	void updateStunFlee(
		float deltaTime,
		const Map& map,
		const Player& player,
		const EnemyConfig& config);
	void updateStunRespawn(
		float deltaTime,
		const Map& map,
		const Player& player,
		const EnemyConfig& config,
		const NavGrid* navGrid);
	void relocate(float x, float y, const Map& map);
	void resetCombatState();

	float distToPlayer(const Player& player) const;

	EnemyType mType = EnemyType::Spirit;
	bool mActive = true;
	float mX = 0.0f;
	float mY = 0.0f;
	float mFacingDeg = 0.0f;
	float mHitboxHalfW = 40.0f;
	float mHitboxHalfH = 40.0f;

	Sprite* mSprite = nullptr;
	Sprite* mSprite_right = nullptr;
	Sprite* mSprite_left = nullptr;

	// Spirit
	float mWanderDirX = 1.0f;
	float mWanderDirY = 0.0f;
	float mWanderTimer = 0.0f;
	bool mSpiritFleeing = false;
	float mSpiritFleeTimer = 0.0f;

	// Space stun: flee or respawn (see tryTriggerStunReaction)
	float mHomeSpawnX = 0.0f;
	float mHomeSpawnY = 0.0f;
	float mStunReactCooldown = 0.0f;
	bool mStunFleeing = false;
	float mStunFleeTimer = 0.0f;
	bool mRespawning = false;
	float mRespawnTimer = 0.0f;

	// Hidden
	float mStealCooldown = 0.0f;
	bool mHiddenRevealed = false;

	// Revenant
	bool mRevenantAggro = false;
	bool mRevenantCharging = false;

	// Shared
	float mHealthDamageCooldown = 0.0f;
	float mStunTimer = 0.0f;
	bool mAlerted = false;
	float mAlertHoldTimer = 0.0f;
	bool mHeardWalkLastFrame = false;
	bool mHeardSprintLastFrame = false;

	std::vector<std::pair<float, float>> mPathWaypoints;
	std::size_t mPathIndex = 0;
	float mPathGoalX = 0.0f;
	float mPathGoalY = 0.0f;
	float mPathReplanTimer = 0.0f;
};
