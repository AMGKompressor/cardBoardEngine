// cardBoard -- enemy AI implementation
#include "Enemy.h"

#include "Collision2D.h"
#include "Map/Map.h"
#include "Player/Player.h"

#include "renderer.h"
#include "sprite.h"

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

		bool insideCone(
			float ox, float oy,
			float tx, float ty,
			float facingDeg,
			float halfAngleDeg,
			float range)
		{
			const float dx = tx - ox;
			const float dy = ty - oy;
			const float distSq = dx * dx + dy * dy;
			if (distSq > range * range)
			{
				return false;
			}
			const float angleToDeg = -std::atan2(dy, dx) * 57.2957795f;
			const float delta = shortestAngleDeltaDegrees(facingDeg, angleToDeg);
			return std::fabs(delta) <= halfAngleDeg;
		}
	}

	// lifecycle
	bool Enemy::initialize(
		Renderer& renderer,
		const EnemyConfig& config,
		float spawnX,
		float spawnY)
	{
		shutdown();

		mConfig = config;
		mX = spawnX;
		mY = spawnY;
		mHealth = mConfig.maxHealth;
		mState = EnemyState::Roaming;
		mFacingDeg = 0.0f;

		mSprite = renderer.createSprite("textures/board8x8.png");
		if (mSprite == nullptr)
		{
			return false;
		}

		const float scale =
			mConfig.desiredSpriteBoxSize / static_cast<float>(mSprite->getWidth());
		mSprite->setScale(scale);
		mSprite->setRedTint(mConfig.bodyTintR);
		mSprite->setGreenTint(mConfig.bodyTintG);
		mSprite->setBlueTint(mConfig.bodyTintB);
		mSprite->setX(static_cast<int>(mX));
		mSprite->setY(static_cast<int>(mY));

		return true;
	}

	void Enemy::shutdown()
	{
		delete mSprite;
		mSprite = nullptr;
	}

	// update
	void Enemy::update(float deltaTime, const Map& map, Player& player)
	{
		if (mState == EnemyState::Dead)
		{
			return;
		}

		if (mStunCooldownTimer > 0.0f)
		{
			mStunCooldownTimer = std::max(0.0f, mStunCooldownTimer - deltaTime);
		}

		tryApplyStun(player);

		switch (mState)
		{
		case EnemyState::Stunned:
			updateStunned(deltaTime);
			break;
		case EnemyState::Chasing:
			updateChasing(deltaTime, map, player);
			break;
		case EnemyState::Roaming:
		default:
			updateRoaming(deltaTime, map, player);
			break;
		}

		if (mState != EnemyState::Stunned && mState != EnemyState::Dead)
		{
			tryDealContactDamage(deltaTime, player);
		}

		if (mSprite != nullptr)
		{
			mSprite->setX(static_cast<int>(mX));
			mSprite->setY(static_cast<int>(mY));
			mSprite->setAngle(mFacingDeg);
		}
	}

	// state: Roaming
	void Enemy::updateRoaming(float deltaTime, const Map& map, const Player& player)
	{
		if (canSeePlayer(player))
		{
			mState = EnemyState::Chasing;
			return;
		}

		const auto& sprintPulses = player.sprintNoisePulses();
		const auto& walkPulses = player.walkNoisePulses();

		auto listenToPulses = [&](const std::vector<NoisePulse>& pulses)
			{
				for (const NoisePulse& pulse : pulses)
				{
					const float dx = pulse.cx - mX;
					const float dy = pulse.cy - mY;
					const float dist = std::sqrt(dx * dx + dy * dy);
					if (dist <= mConfig.hearingRadius)
					{
						mRoamTargetX = pulse.cx;
						mRoamTargetY = pulse.cy;
						mHasRoamTarget = true;
					}
				}
			};

		listenToPulses(sprintPulses);
		listenToPulses(walkPulses);

		if (!mHasRoamTarget)
		{
			return;
		}

		const float dx = mRoamTargetX - mX;
		const float dy = mRoamTargetY - mY;
		const float dist = std::sqrt(dx * dx + dy * dy);

		if (dist < 8.0f)
		{
			mHasRoamTarget = false;
			return;
		}

		const float invDist = 1.0f / dist;
		applyMovement(dx * invDist, dy * invDist, mConfig.roamSpeed, deltaTime, map);
		faceDirection(dx, dy, deltaTime);
	}

	// state: Chasing
	void Enemy::updateChasing(float deltaTime, const Map& map, const Player& player)
	{
		const float dx = player.x() - mX;
		const float dy = player.y() - mY;
		const float dist = std::sqrt(dx * dx + dy * dy);

		if (dist < 1.0f)
		{
			return;
		}

		const float invDist = 1.0f / dist;
		applyMovement(dx * invDist, dy * invDist, mConfig.chaseSpeed, deltaTime, map);
		faceDirection(dx, dy, deltaTime);

		if (!canSeePlayer(player) && !canHearPlayer(player))
		{
			mState = EnemyState::Roaming;
			mRoamTargetX = player.x();
			mRoamTargetY = player.y();
			mHasRoamTarget = true;
		}
	}

	// state: Stunned
	void Enemy::updateStunned(float deltaTime)
	{
		mStunTimer = std::max(0.0f, mStunTimer - deltaTime);
		if (mStunTimer <= 0.0f)
		{
			mStunCooldownTimer = mConfig.stunCooldown;
			mState = EnemyState::Roaming;
		}
	}

	// movement helpers
	void Enemy::applyMovement(
		float dx, float dy,
		float speed,
		float deltaTime,
		const Map& map)
	{
		float nextX = mX + dx * speed * deltaTime;
		float nextY = mY + dy * speed * deltaTime;

		float worldHalfX = 0.0f;
		float worldHalfY = 0.0f;
		orientedRectWorldAabbHalfExtents(
			mConfig.hitboxHalfW,
			mConfig.hitboxHalfH,
			mFacingDeg,
			worldHalfX,
			worldHalfY);

		map.constrainPlayerCenter(
			nextX,
			nextY,
			worldHalfX,
			worldHalfY,
			mConfig.hitboxHalfW,
			mConfig.hitboxHalfH,
			mFacingDeg);

		mX = nextX;
		mY = nextY;
	}

	void Enemy::faceDirection(float dx, float dy, float deltaTime)
	{
		if (dx * dx + dy * dy < 1.0f)
		{
			return;
		}

		const float targetDeg = -std::atan2(dy, dx) * 57.2957795f;
		const float maxStep = mConfig.turnRateDegPerSec * deltaTime;
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

	// perception helpers
	bool Enemy::canHearPlayer(const Player& player) const
	{
		const auto check = [&](const std::vector<NoisePulse>& pulses) -> bool
			{
				for (const NoisePulse& pulse : pulses)
				{
					const float dx = pulse.cx - mX;
					const float dy = pulse.cy - mY;
					const float dist = std::sqrt(dx * dx + dy * dy);
					if (dist <= mConfig.hearingRadius)
					{
						return true;
					}
				}
				return false;
			};

		return check(player.sprintNoisePulses()) || check(player.walkNoisePulses());
	}

	bool Enemy::canSeePlayer(const Player& player) const
	{
		const float dx = player.x() - mX;
		const float dy = player.y() - mY;
		const float dist = std::sqrt(dx * dx + dy * dy);
		return dist <= mConfig.sightRadius;
	}

	// stun
	void Enemy::tryApplyStun(const Player& player)
	{
		if (mState == EnemyState::Stunned || mStunCooldownTimer > 0.0f)
		{
			return;
		}

		const FlashlightStunQuery query = player.flashlightStunQuery();
		if (!query.active)
		{
			return;
		}

		if (insideCone(
			query.originX, query.originY,
			mX, mY,
			query.facingDeg,
			query.halfAngleDeg,
			query.range))
		{
			mState = EnemyState::Stunned;
			mStunTimer = mConfig.stunDuration;
		}
	}

	// damage
	void Enemy::tryDealContactDamage(float deltaTime, Player& player)
	{
		const float dx = player.x() - mX;
		const float dy = player.y() - mY;
		const float dist = std::sqrt(dx * dx + dy * dy);
		if (dist <= mConfig.touchDamageRadius)
		{
			player.applyDamage(mConfig.damagePerSecond * deltaTime);
		}
	}

	// drawing
	void Enemy::drawSprite(Renderer& renderer) const
	{
		if (mState == EnemyState::Dead || mSprite == nullptr)
		{
			return;
		}

		if (mState == EnemyState::Stunned)
		{
			mSprite->setRedTint(1.0f);
			mSprite->setGreenTint(1.0f);
			mSprite->setBlueTint(1.0f);
		}
		else
		{
			mSprite->setRedTint(mConfig.bodyTintR);
			mSprite->setGreenTint(mConfig.bodyTintG);
			mSprite->setBlueTint(mConfig.bodyTintB);
		}

		mSprite->draw(renderer);
	}

	void Enemy::drawDebug(Renderer& renderer) const
	{
		if (!mShowDebug || mState == EnemyState::Dead)
		{
			return;
		}

		drawWorldCircleOutline(renderer, mX, mY, mConfig.hearingRadius, 32,
			1.0f, 0.9f, 0.1f, 0.25f);

		drawWorldCircleOutline(renderer, mX, mY, mConfig.sightRadius, 24,
			1.0f, 0.2f, 0.2f, 0.35f);

		drawWorldCircleOutline(renderer, mX, mY, mConfig.touchDamageRadius, 16,
			1.0f, 1.0f, 1.0f, 0.55f);
	}
}