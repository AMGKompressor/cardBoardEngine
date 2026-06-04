#pragma once

enum class EnemyType
{
	Spirit,
	Hidden,
	Revenant
};

struct EnemySpawnSpec
{
	EnemyType type = EnemyType::Spirit;
	float worldX = 0.0f;
	float worldY = 0.0f;
};

struct EnemyConfig
{
	float hitboxHalfW = 40.0f;
	float hitboxHalfH = 40.0f;
	float spriteBoxSize = 64.0f;

	// Footstep hearing (synced from PlayerConfig::footstep)
	float hearRadiusWalk = 140.0f;
	float hearRadiusSprint = 320.0f;
	float playerAwarenessRadius = 320.0f;
	float alertHoldSeconds = 2.5f;

	// BFS pathfinding (NavGrid)
	float pathReplanSeconds = 0.35f;
	float pathWaypointArrival = 28.0f;

	// Space + flashlight stun beam (cone + range)
	float stunReactMaxDistance = 620.0f;
	float stunReactCooldown = 4.0f;
	float stunFleeDuration = 2.8f;
	float stunFleeSpeedSpirit = 150.0f;
	float stunFleeSpeedRevenant = 300.0f;
	float stunRespawnHideDuration = 0.5f;
	float stunRespawnMinDistFromPlayer = 480.0f;
	float stunRespawnMinDistFromSelf = 260.0f;

	// Spirit — sanity harm when alerted + in aura
	float spiritWanderSpeed = 55.0f;
	float spiritWanderTurnInterval = 2.5f;
	float spiritAuraRadius = 180.0f;
	float spiritSanityDrainPerSecond = 6.0f;
	float spiritApproachSpeed = 80.0f;
	float spiritFleeSpeed = 140.0f;
	float spiritFleeDuration = 2.0f;
	float spiritStunDuration = 1.8f;

	// Hidden — steals when alerted (heard footsteps) and in reach
	float hiddenStealRadius = 140.0f;
	float hiddenStealCooldown = 4.0f;
	float hiddenApproachSpeed = 95.0f;
	float hiddenShimmerAlpha = 0.35f;
	float hiddenSanityOnSteal = 8.0f;
	float hiddenHealthDamageOnSteal = 14.0f;   // health, not sanity
	float hiddenContactHealthPerSecond = 10.0f;

	// Revenant — charges on sprint footsteps, investigates walk; contact hurts health
	float revenantPatrolSpeed = 70.0f;
	float revenantInvestigateSpeed = 120.0f;
	float revenantChargeSpeed = 320.0f;
	float revenantTurnRateDegPerSec = 48.0f;
	float revenantContactRadius = 70.0f;
	float revenantHealthDamagePerSecond = 18.0f;
	float revenantContactBurstDamage = 12.0f;
	float revenantContactBurstCooldown = 1.2f;
	float revenantStunDuration = 1.5f;
};
