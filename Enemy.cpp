#include "Enemy.h"

#include "NavGrid.h"
#include "Collision2D_1.h"
#include "Item.h"
#include "Map_1.h"
#include "Player_1.h"
#include "logmanager.h"
#include "renderer.h"
#include "sprite.h"

#include "imgui.h"

#include <algorithm>
#include <vector>
#include <cmath>
#include <cstdlib>

namespace
{
	struct FootstepHearing
	{
		bool heardWalk = false;
		bool heardSprint = false;
	};

	float distSq(float ax, float ay, float bx, float by)
	{
		const float dx = bx - ax;
		const float dy = by - ay;
		return dx * dx + dy * dy;
	}

	// Must match vision_mask.frag cone math (uses uFacingDeg with 90 - facing look vector).
	bool pointInStunCone(
		const FlashlightStunQuery& query,
		float px,
		float py)
	{
		if (!query.active)
		{
			return false;
		}

		const float dx = px - query.originX;
		const float dy = py - query.originY;
		const float distSqVal = dx * dx + dy * dy;
		const float range = query.range;
		if (distSqVal > range * range)
		{
			return false;
		}

		const float degToRad = 0.0174532925f;
		const float ang = std::atan2(dy, dx);
		const float lookRad = (90.0f - query.facingDeg) * degToRad;
		float deltaRad = ang - lookRad;
		deltaRad = std::atan2(std::sin(deltaRad), std::cos(deltaRad));
		const float halfRad = query.halfAngleDeg * degToRad;
		return std::fabs(deltaRad) <= halfRad;
	}

	void pickWanderDirection(float& outX, float& outY)
	{
		const float angle = static_cast<float>((std::rand() % 6283) / 1000.0);
		outX = std::cos(angle);
		outY = std::sin(angle);
	}

	// Hidden / Revenant deal health damage; all enemy types add sanity drain rate when harming.
	void damagePlayerHealth(Player& player, float amount)
	{
		if (amount <= 0.0f)
		{
			return;
		}
		player.playerHealth = std::max(0.0f, player.playerHealth - amount);
	}

	FootstepHearing queryFootstepHearing(
		float ex,
		float ey,
		const Player& player,
		const EnemyConfig& config)
	{
		FootstepHearing hearing;
		const float walkSq = config.hearRadiusWalk * config.hearRadiusWalk;
		const float sprintSq = config.hearRadiusSprint * config.hearRadiusSprint;

		for (const NoisePulse& pulse : player.walkNoisePulses())
		{
			if (distSq(ex, ey, pulse.cx, pulse.cy) <= walkSq)
			{
				hearing.heardWalk = true;
			}
		}

		for (const NoisePulse& pulse : player.sprintNoisePulses())
		{
			if (distSq(ex, ey, pulse.cx, pulse.cy) <= sprintSq)
			{
				hearing.heardSprint = true;
			}
		}

		const float playerDistSq = distSq(ex, ey, player.x(), player.y());
		const bool playerMoving = player.moveSpeed() > 8.0f;

		if (player.isRunning && playerMoving && playerDistSq <= sprintSq)
		{
			hearing.heardSprint = true;
		}
		else if (playerMoving && playerDistSq <= walkSq)
		{
			hearing.heardWalk = true;
		}

		return hearing;
	}
}

Enemy::Enemy() = default;

Enemy::~Enemy()
{
	shutdown();
}

bool Enemy::initialize(
	EnemyType type,
	Renderer& renderer,
	const EnemyConfig& config,
	float spawnX,
	float spawnY)
{
	shutdown();

	mType = type;
	mActive = true;
	mX = spawnX;
	mY = spawnY;
	mHomeSpawnX = spawnX;
	mHomeSpawnY = spawnY;
	mHitboxHalfW = config.hitboxHalfW;
	mHitboxHalfH = config.hitboxHalfH;
	mFacingDeg = 0.0f;
	mStunTimer = 0.0f;
	mSpiritFleeing = false;
	mSpiritFleeTimer = 0.0f;
	mStunReactCooldown = 0.5f;
	mStunFleeing = false;
	mStunFleeTimer = 0.0f;
	mRespawning = false;
	mRespawnTimer = 0.0f;
	mRevenantAggro = false;
	mRevenantCharging = false;
	mStealCooldown = 1.0f;
	mHealthDamageCooldown = 0.0f;
	mHiddenRevealed = false;
	mAlerted = false;
	mAlertHoldTimer = 0.0f;
	mHeardWalkLastFrame = false;
	mHeardSprintLastFrame = false;
	clearPath();
	pickWanderDirection(mWanderDirX, mWanderDirY);
	mWanderTimer = 0.0f;

	mSprite = renderer.createSprite("/assets/textures/board8x8.png");
	if (mSprite == nullptr)
	{
		return false;
	}

	const float scale =
		config.spriteBoxSize / static_cast<float>(mSprite->getWidth());
	mSprite->setScale(scale);

	switch (mType)
	{
	case EnemyType::Spirit:
		mSprite->setRedTint(0.55f);
		mSprite->setGreenTint(0.75f);
		mSprite->setBlueTint(1.0f);
		mSprite->setAlpha(0.55f);
		break;
	case EnemyType::Hidden:
		mSprite->setRedTint(0.35f);
		mSprite->setGreenTint(0.35f);
		mSprite->setBlueTint(0.40f);
		mSprite->setAlpha(0.0f);
		break;
	case EnemyType::Revenant:
		mSprite->setRedTint(0.95f);
		mSprite->setGreenTint(0.15f);
		mSprite->setBlueTint(0.10f);
		mSprite->setAlpha(0.95f);
		break;
	}

	mSprite->setX(static_cast<int>(mX));
	mSprite->setY(static_cast<int>(mY));
	mSprite->setAngle(mFacingDeg);
	return true;
}

void Enemy::shutdown()
{
	delete mSprite;
	mSprite = nullptr;
	clearPath();
}

void Enemy::clearPath()
{
	mPathWaypoints.clear();
	mPathIndex = 0;
	mPathGoalX = 0.0f;
	mPathGoalY = 0.0f;
	mPathReplanTimer = 0.0f;
}

void Enemy::moveAlongPathTo(
	float deltaTime,
	const Map& map,
	const NavGrid& navGrid,
	float goalX,
	float goalY,
	float speed,
	float turnRateDegPerSec,
	bool faceMovement,
	const EnemyConfig& config)
{
	const float goalDeltaSq =
		(goalX - mPathGoalX) * (goalX - mPathGoalX)
		+ (goalY - mPathGoalY) * (goalY - mPathGoalY);

	mPathReplanTimer -= deltaTime;
	const bool needReplan =
		mPathWaypoints.empty()
		|| mPathReplanTimer <= 0.0f
		|| goalDeltaSq > navGrid.cellSize() * navGrid.cellSize();

	if (needReplan && navGrid.isBuilt())
	{
		mPathWaypoints.clear();
		mPathIndex = 0;
		if (navGrid.findPath(mX, mY, goalX, goalY, mPathWaypoints))
		{
			mPathGoalX = goalX;
			mPathGoalY = goalY;
			mPathReplanTimer = config.pathReplanSeconds;
			if (mPathWaypoints.size() > 1)
			{
				mPathIndex = 1;
			}
			else
			{
				mPathIndex = 0;
			}
		}
	}

	if (mPathWaypoints.empty() || mPathIndex >= mPathWaypoints.size())
	{
		moveWithCollision(
			deltaTime,
			map,
			goalX - mX,
			goalY - mY,
			speed,
			turnRateDegPerSec,
			faceMovement);
		return;
	}

	const float targetX = mPathWaypoints[mPathIndex].first;
	const float targetY = mPathWaypoints[mPathIndex].second;
	const float dx = targetX - mX;
	const float dy = targetY - mY;
	const float arrival = config.pathWaypointArrival;

	if (dx * dx + dy * dy <= arrival * arrival)
	{
		++mPathIndex;
		if (mPathIndex >= mPathWaypoints.size())
		{
			moveWithCollision(
				deltaTime,
				map,
				goalX - mX,
				goalY - mY,
				speed,
				turnRateDegPerSec,
				faceMovement);
			return;
		}
	}

	const float nextX = mPathWaypoints[mPathIndex].first;
	const float nextY = mPathWaypoints[mPathIndex].second;
	moveWithCollision(
		deltaTime,
		map,
		nextX - mX,
		nextY - mY,
		speed,
		turnRateDegPerSec,
		faceMovement);
}

float Enemy::distToPlayer(const Player& player) const
{
	return std::sqrt(distSq(mX, mY, player.x(), player.y()));
}

bool Enemy::isWithinPlayerAwareness(const Player& player, const EnemyConfig& config) const
{
	const float radius = config.playerAwarenessRadius;
	return distSq(mX, mY, player.x(), player.y()) <= radius * radius;
}

bool Enemy::isWithinStunReactRange(const Player& player, const EnemyConfig& config) const
{
	const FlashlightStunQuery query = player.flashlightStunQuery();
	if (!query.active)
	{
		return false;
	}

	const float maxDist = std::min(config.stunReactMaxDistance, query.range);
	return distSq(mX, mY, player.x(), player.y()) <= maxDist * maxDist;
}

void Enemy::resetCombatState()
{
	mAlerted = false;
	mAlertHoldTimer = 0.0f;
	mHeardWalkLastFrame = false;
	mHeardSprintLastFrame = false;
	mRevenantAggro = false;
	mRevenantCharging = false;
	mSpiritFleeing = false;
	mSpiritFleeTimer = 0.0f;
	mHiddenRevealed = false;
	clearPath();
}

void Enemy::relocate(float x, float y, const Map& map)
{
	mX = x;
	mY = y;

	float worldHalfX = 0.0f;
	float worldHalfY = 0.0f;
	orientedRectWorldAabbHalfExtents(
		mHitboxHalfW, mHitboxHalfH, mFacingDeg, worldHalfX, worldHalfY);

	map.constrainPlayerCenter(
		mX,
		mY,
		worldHalfX,
		worldHalfY,
		mHitboxHalfW,
		mHitboxHalfH,
		mFacingDeg);

	if (mSprite != nullptr)
	{
		mSprite->setX(static_cast<int>(mX));
		mSprite->setY(static_cast<int>(mY));
	}
}

// Flashlight on + Space, enemy in stun cone: Spirit/Revenant flee, Hidden teleports away.
void Enemy::tryTriggerStunReaction(
	const Player& player,
	const EnemyConfig& config,
	const NavGrid* navGrid)
{
	(void)navGrid;

	if (mStunReactCooldown > 0.0f || mStunFleeing || mRespawning)
	{
		return;
	}

	if (!isInFlashlightStun(player) || !isWithinStunReactRange(player, config))
	{
		return;
	}

	mStunReactCooldown = config.stunReactCooldown;
	resetCombatState();

	switch (mType)
	{
	case EnemyType::Spirit:
		mStunFleeing = true;
		mStunFleeTimer = config.stunFleeDuration;
		LogManager::getInstance().log("Spirit flees from your stun beam.");
		break;
	case EnemyType::Hidden:
		mRespawning = true;
		mRespawnTimer = config.stunRespawnHideDuration;
		if (mSprite != nullptr)
		{
			mSprite->setAlpha(0.0f);
		}
		LogManager::getInstance().log("The Hidden vanishes...");
		break;
	case EnemyType::Revenant:
		mStunFleeing = true;
		mStunFleeTimer = config.stunFleeDuration;
		LogManager::getInstance().log("Revenant retreats from the stun beam.");
		break;
	}
}

void Enemy::updateStunFlee(
	float deltaTime,
	const Map& map,
	const Player& player,
	const EnemyConfig& config)
{
	// Run away from player for a short time.
	mStunFleeTimer -= deltaTime;
	if (mStunFleeTimer <= 0.0f)
	{
		mStunFleeing = false;
	}

	const float fleeSpeed = (mType == EnemyType::Revenant)
		? config.stunFleeSpeedRevenant
		: config.stunFleeSpeedSpirit;

	moveWithCollision(
		deltaTime,
		map,
		mX - player.x(),
		mY - player.y(),
		fleeSpeed,
		360.0f,
		true);
}

void Enemy::updateStunRespawn(
	float deltaTime,
	const Map& map,
	const Player& player,
	const EnemyConfig& config,
	const NavGrid* navGrid)
{
	mRespawnTimer -= deltaTime;
	if (mRespawnTimer > 0.0f)
	{
		return;
	}

	// Pick a new walkable cell via NavGrid (Hidden after stun).
	float newX = mHomeSpawnX;
	float newY = mHomeSpawnY;
	bool found = false;

	if (navGrid != nullptr && navGrid->isBuilt())
	{
		found = navGrid->pickRelocatedPosition(
			player.x(),
			player.y(),
			config.stunRespawnMinDistFromPlayer,
			mX,
			mY,
			config.stunRespawnMinDistFromSelf,
			newX,
			newY);
	}

	if (!found)
	{
		newX = mHomeSpawnX;
		newY = mHomeSpawnY;
	}

	relocate(newX, newY, map);
	mRespawning = false;
	resetCombatState();

	if (mSprite != nullptr)
	{
		mSprite->setAlpha(0.0f);
	}

	LogManager::getInstance().log("The Hidden reappeared elsewhere in the map.");
}

void Enemy::refreshAlertFromFootsteps(
	const Player& player,
	const EnemyConfig& config,
	float deltaTime)
{
	if (!isWithinPlayerAwareness(player, config))
	{
		mAlertHoldTimer = std::max(0.0f, mAlertHoldTimer - deltaTime);
		if (mAlertHoldTimer <= 0.0f)
		{
			mAlerted = false;
		}
		mHeardWalkLastFrame = false;
		mHeardSprintLastFrame = false;
		return;
	}

	const FootstepHearing hearing = queryFootstepHearing(mX, mY, player, config);
	mHeardWalkLastFrame = hearing.heardWalk;
	mHeardSprintLastFrame = hearing.heardSprint;

	if (hearing.heardWalk || hearing.heardSprint)
	{
		mAlerted = true;
		mAlertHoldTimer = config.alertHoldSeconds;
	}
	else
	{
		mAlertHoldTimer = std::max(0.0f, mAlertHoldTimer - deltaTime);
		if (mAlertHoldTimer <= 0.0f)
		{
			mAlerted = false;
		}
	}
}

bool Enemy::isInFlashlightStun(const Player& player) const
{
	return pointInStunCone(player.flashlightStunQuery(), mX, mY);
}

void Enemy::applyStun(float duration)
{
	mStunTimer = std::max(mStunTimer, duration);
}

bool Enemy::isStunned(float deltaTime, const EnemyConfig& config)
{
	(void)config;
	if (mStunTimer > 0.0f)
	{
		mStunTimer = std::max(0.0f, mStunTimer - deltaTime);
		return true;
	}
	return false;
}

void Enemy::moveWithCollision(
	float deltaTime,
	const Map& map,
	float dirX,
	float dirY,
	float speed,
	float turnRateDegPerSec,
	bool faceMovement)
{
	const float lenSq = dirX * dirX + dirY * dirY;
	if (lenSq < 0.0001f)
	{
		return;
	}

	const float invLen = 1.0f / std::sqrt(lenSq);
	dirX *= invLen;
	dirY *= invLen;

	if (faceMovement && turnRateDegPerSec > 0.0f)
	{
		const float targetDeg = -std::atan2(dirY, dirX) * 57.2957795f;
		const float maxStep = turnRateDegPerSec * deltaTime;
		const float delta = shortestAngleDeltaDegrees(mFacingDeg, targetDeg);
		if (std::fabs(delta) <= maxStep)
		{
			mFacingDeg = targetDeg;
		}
		else
		{
			mFacingDeg += (delta >= 0.0f) ? maxStep : -maxStep;
		}
		mFacingDeg = wrap360(mFacingDeg);
	}

	const float nextX = mX + dirX * speed * deltaTime;
	const float nextY = mY + dirY * speed * deltaTime;

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

	mX = clampedX;
	mY = clampedY;

	if (mSprite != nullptr)
	{
		mSprite->setX(static_cast<int>(mX));
		mSprite->setY(static_cast<int>(mY));
		mSprite->setAngle(mFacingDeg);
	}
}

void Enemy::updateSpirit(
	float deltaTime,
	const Map& map,
	const Player& player,
	const EnemyConfig& config,
	const NavGrid* navGrid)
{
	if (!isWithinPlayerAwareness(player, config))
	{
		if (mSprite != nullptr)
		{
			mSprite->setAlpha(0.35f);
		}
		return;
	}

	if (mAlerted)
	{
		if (navGrid != nullptr && navGrid->isBuilt())
		{
			moveAlongPathTo(
				deltaTime,
				map,
				*navGrid,
				player.x(),
				player.y(),
				config.spiritApproachSpeed,
				90.0f,
				true,
				config);
		}
		else
		{
			moveWithCollision(
				deltaTime,
				map,
				player.x() - mX,
				player.y() - mY,
				config.spiritApproachSpeed,
				90.0f,
				true);
		}
	}
	else
	{
		mWanderTimer -= deltaTime;
		if (mWanderTimer <= 0.0f)
		{
			pickWanderDirection(mWanderDirX, mWanderDirY);
			mWanderTimer = config.spiritWanderTurnInterval;
		}

		moveWithCollision(
			deltaTime,
			map,
			mWanderDirX,
			mWanderDirY,
			config.spiritWanderSpeed,
			90.0f,
			true);
	}

	const float dist = distToPlayer(player);
	const bool inHarmRange = mAlerted && dist <= config.spiritAuraRadius;
	if (inHarmRange)
	{
		const float t = 1.0f - (dist / config.spiritAuraRadius);
		Player& mutablePlayer = const_cast<Player&>(player);
		mutablePlayer.addEnemySanityDrain(config.spiritSanityDrainPerSecond * t);

		if (mSprite != nullptr)
		{
			mSprite->setAlpha(0.45f + 0.35f * t);
		}
	}
	else if (mSprite != nullptr)
	{
		mSprite->setAlpha(mAlerted ? 0.65f : 0.45f);
	}
}

void Enemy::updateHidden(
	float deltaTime,
	const Map& map,
	const Player& player,
	const std::vector<Item*>& worldItems,
	const EnemyConfig& config,
	const NavGrid* navGrid)
{
	if (!isWithinPlayerAwareness(player, config))
	{
		if (mSprite != nullptr)
		{
			mSprite->setAlpha(0.0f);
		}
		return;
	}

	mStealCooldown = std::max(0.0f, mStealCooldown - deltaTime);

	const bool revealed =
		isInFlashlightStun(player)
		|| (player.flashlightOn()
			&& distToPlayer(player) <= config.hearRadiusWalk);
	mHiddenRevealed = revealed;

	if (mSprite != nullptr)
	{
		mSprite->setAlpha(revealed ? config.hiddenShimmerAlpha : 0.0f);
	}

	if (mAlerted && mStealCooldown <= 0.0f)
	{
		const float dist = distToPlayer(player);
		if (dist <= config.hiddenStealRadius)
		{
			for (Item* item : worldItems)
			{
				if (item == nullptr)
				{
					continue;
				}
				if (item->trySteal(mX, mY, config.hiddenStealRadius))
				{
					Player& mutablePlayer = const_cast<Player&>(player);
					mutablePlayer.sanityPercentage = std::max(
						0.0f,
						mutablePlayer.sanityPercentage - config.hiddenSanityOnSteal);
					damagePlayerHealth(mutablePlayer, config.hiddenHealthDamageOnSteal);
					mStealCooldown = config.hiddenStealCooldown;
					LogManager::getInstance().log(
						"The Hidden stole a battery and wounded you.");
					break;
				}
			}
		}
	}

	if (mAlerted)
	{
		const float dist = distToPlayer(player);
		if (dist <= config.hiddenStealRadius)
		{
			Player& mutablePlayer = const_cast<Player&>(player);
			damagePlayerHealth(
				mutablePlayer,
				config.hiddenContactHealthPerSecond * deltaTime);
			mutablePlayer.addEnemySanityDrain(config.hiddenSanityDrainPerSecond);
		}
	}

	if (mAlerted && !revealed)
	{
		if (navGrid != nullptr && navGrid->isBuilt())
		{
			moveAlongPathTo(
				deltaTime,
				map,
				*navGrid,
				player.x(),
				player.y(),
				config.hiddenApproachSpeed,
				60.0f,
				false,
				config);
		}
		else
		{
			moveWithCollision(
				deltaTime,
				map,
				player.x() - mX,
				player.y() - mY,
				config.hiddenApproachSpeed,
				60.0f,
				false);
		}
	}
}

void Enemy::updateRevenant(
	float deltaTime,
	const Map& map,
	const Player& player,
	const EnemyConfig& config,
	const NavGrid* navGrid)
{
	if (!isWithinPlayerAwareness(player, config))
	{
		mRevenantAggro = false;
		mRevenantCharging = false;
		return;
	}

	if (mHeardSprintLastFrame)
	{
		if (!mRevenantAggro)
		{
			LogManager::getInstance().log("A Revenant heard you sprinting!");
		}
		mRevenantAggro = true;
		mRevenantCharging = true;
	}
	else if (mHeardWalkLastFrame)
	{
		if (!mRevenantAggro)
		{
			LogManager::getInstance().log("A Revenant heard footsteps...");
		}
		mRevenantAggro = true;
		mRevenantCharging = false;
	}
	else if (!mAlerted)
	{
		mRevenantAggro = false;
		mRevenantCharging = false;
	}

	float speed = config.revenantPatrolSpeed;

	mHealthDamageCooldown = std::max(0.0f, mHealthDamageCooldown - deltaTime);

	if (mRevenantAggro)
	{
		speed = mRevenantCharging
			? config.revenantChargeSpeed
			: config.revenantInvestigateSpeed;

		const float contactSq = config.revenantContactRadius * config.revenantContactRadius;
		if (distSq(mX, mY, player.x(), player.y()) <= contactSq)
		{
			Player& mutablePlayer = const_cast<Player&>(player);
			damagePlayerHealth(
				mutablePlayer,
				config.revenantHealthDamagePerSecond * deltaTime);
			mutablePlayer.addEnemySanityDrain(config.revenantSanityDrainPerSecond);
			if (mHealthDamageCooldown <= 0.0f)
			{
				damagePlayerHealth(mutablePlayer, config.revenantContactBurstDamage);
				mHealthDamageCooldown = config.revenantContactBurstCooldown;
			}
		}

		if (navGrid != nullptr && navGrid->isBuilt())
		{
			moveAlongPathTo(
				deltaTime,
				map,
				*navGrid,
				player.x(),
				player.y(),
				speed,
				config.revenantTurnRateDegPerSec,
				true,
				config);
		}
		else
		{
			moveWithCollision(
				deltaTime,
				map,
				player.x() - mX,
				player.y() - mY,
				speed,
				config.revenantTurnRateDegPerSec,
				true);
		}
		return;
	}

	mWanderTimer -= deltaTime;
	if (mWanderTimer <= 0.0f)
	{
		pickWanderDirection(mWanderDirX, mWanderDirY);
		mWanderTimer = 3.0f;
	}
	moveWithCollision(
		deltaTime,
		map,
		mWanderDirX,
		mWanderDirY,
		speed,
		config.revenantTurnRateDegPerSec,
		true);
}

void Enemy::update(
	float deltaTime,
	const Map& map,
	const Player& player,
	const std::vector<Item*>& worldItems,
	const EnemyConfig& config,
	const NavGrid* navGrid)
{
	if (!mActive || mSprite == nullptr)
	{
		return;
	}

	refreshAlertFromFootsteps(player, config, deltaTime);

	mStunReactCooldown = std::max(0.0f, mStunReactCooldown - deltaTime);

	// Stun reactions run before normal enemy AI.
	if (mRespawning)
	{
		updateStunRespawn(deltaTime, map, player, config, navGrid);
		return;
	}

	tryTriggerStunReaction(player, config, navGrid);

	if (mStunFleeing)
	{
		updateStunFlee(deltaTime, map, player, config);
		return;
	}

	switch (mType)
	{
	case EnemyType::Spirit:
		updateSpirit(deltaTime, map, player, config, navGrid);
		break;
	case EnemyType::Hidden:
		updateHidden(deltaTime, map, player, worldItems, config, navGrid);
		break;
	case EnemyType::Revenant:
		updateRevenant(deltaTime, map, player, config, navGrid);
		break;
	}
}

void Enemy::draw(Renderer& renderer) const
{
	if (!mActive || mSprite == nullptr || mRespawning)
	{
		return;
	}

	if (mType == EnemyType::Hidden && mSprite->getAlpha() <= 0.01f)
	{
		return;
	}

	mSprite->draw(renderer);
}

void Enemy::drawPathDebug(Renderer& renderer) const
{
	if (mPathWaypoints.size() < 2)
	{
		if (mPathWaypoints.size() == 1)
		{
			const float wx = mPathWaypoints[0].first;
			const float wy = mPathWaypoints[0].second;
			renderer.drawWorldAxisAlignedQuad(wx, wy, 10.0f, 10.0f, 0.2f, 0.9f, 1.0f, 0.9f);
		}
		return;
	}

	std::vector<float> lineVerts;
	lineVerts.reserve(mPathWaypoints.size() * 4);
	for (std::size_t i = 0; i + 1 < mPathWaypoints.size(); ++i)
	{
		lineVerts.push_back(mPathWaypoints[i].first);
		lineVerts.push_back(mPathWaypoints[i].second);
		lineVerts.push_back(mPathWaypoints[i + 1].first);
		lineVerts.push_back(mPathWaypoints[i + 1].second);
	}

	const int segmentCount = static_cast<int>(mPathWaypoints.size()) - 1;
	renderer.drawWorldLineSegments(
		lineVerts.data(),
		segmentCount,
		1.0f,
		0.95f,
		0.15f,
		0.85f);

	for (std::size_t i = 0; i < mPathWaypoints.size(); ++i)
	{
		const float wx = mPathWaypoints[i].first;
		const float wy = mPathWaypoints[i].second;
		const bool isCurrent = (i == mPathIndex);
		if (isCurrent)
		{
			renderer.drawWorldAxisAlignedQuad(wx, wy, 12.0f, 12.0f, 0.2f, 0.9f, 1.0f, 0.95f);
		}
		else
		{
			renderer.drawWorldAxisAlignedQuad(wx, wy, 7.0f, 7.0f, 1.0f, 0.85f, 0.1f, 0.75f);
		}
	}

	renderer.drawWorldAxisAlignedQuad(mX, mY, 9.0f, 9.0f, 1.0f, 0.2f, 0.2f, 0.9f);
}

void Enemy::debugDraw() const
{
	const char* typeLabel = "Unknown";
	switch (mType)
	{
	case EnemyType::Spirit: typeLabel = "Spirit"; break;
	case EnemyType::Hidden: typeLabel = "Hidden"; break;
	case EnemyType::Revenant: typeLabel = "Revenant"; break;
	}

	const char* harmLabel = "sanity";
	switch (mType)
	{
	case EnemyType::Spirit: harmLabel = "sanity (aura)"; break;
	case EnemyType::Hidden: harmLabel = "health + sanity drain (close / steal)"; break;
	case EnemyType::Revenant: harmLabel = "health + sanity drain (contact)"; break;
	}

	const char* stunState = "idle";
	if (mRespawning)
	{
		stunState = "respawning";
	}
	else if (mStunFleeing)
	{
		stunState = "fleeing";
	}

	ImGui::Text(
		"%s — alerted: %s, harm: %s, stun: %s",
		typeLabel,
		mAlerted ? "yes" : "no",
		harmLabel,
		stunState);
	ImGui::Text("  at (%.0f, %.0f), path nodes: %d (index %d)",
		mX, mY,
		static_cast<int>(mPathWaypoints.size()),
		static_cast<int>(mPathIndex));
}
