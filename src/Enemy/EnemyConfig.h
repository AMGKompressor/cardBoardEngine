// cardBoard -- tunable enemy stats (mirrors PlayerConfig style)
#pragma once

namespace CardBoard
{
	struct EnemyConfig
	{
		float chaseSpeed = 110.0f;
		float roamSpeed = 45.0f;
		float turnRateDegPerSec = 180.0f;

		float hitboxHalfW = 32.0f;
		float hitboxHalfH = 32.0f;

		float hearingRadius = 340.0f;
		float sightRadius = 220.0f;
		float touchDamageRadius = 52.0f;
		float damagePerSecond = 18.0f;

		float maxHealth = 100.0f;
		float stunDuration = 2.2f;
		float stunCooldown = 0.8f;

		float bodyTintR = 0.72f;
		float bodyTintG = 0.08f;
		float bodyTintB = 0.08f;
		float desiredSpriteBoxSize = 80.0f;
	};
}