// cardBoard — tunable player stats (like a Java config / settings class)
#pragma once

struct FlashlightConfig
{
	float halfAngleDeg = 22.0f;
	float beamRange = 460.0f;
	float featherDeg = 4.0f;
	float maskAlpha = 0.98f;
	float ambientRadius = 72.0f;
	float ambientFeather = 14.0f;
};

struct SanityMeterConfig
{
	float maxCharge = 10.0f;
	float drainPerSecond = .5f;
	float rechargePerSecond = 0.4f;
	

	float screenOffsetX = 10.0f;
	float screenOffsetY = 10.0f;
	float width = 60.0f;
	float height = 8.0f;
	float border = 2.0f;
};

struct HealthMeterConfig
{
	float maxCharge = 100.0f;
	float decay = 0.85f;

	float screenOffsetX = 10.0f;
	float screenOffsetY = 20.0f;
	float width = 100.0f;
	float height = 12.0f;
	float border = 2.0f;
};
struct FlashlightMeterConfig
{
	float maxChargeSeconds = 10.0f;
	float drainPerSecond = 1.0f;
	float rechargePerSecond = 0.75f;
	float minChargeToToggleOn = 0.25f;

	float screenOffsetX = 20.0f;
	float screenOffsetY = 20.0f;
	float width = 180.0f;
	float height = 16.0f;
	float border = 2.0f;
};

struct FlashlightStunConfig
{
	float extraDrainPerSecond = 3.2f;
	float beamHalfAngleMultiplier = 1.35f;
	float beamRangeMultiplier = 1.35f;
	float coneFeatherMultiplier = 0.9f;
};

struct FootstepConfig
{
	float intervalWalk = 0.42f;
	float intervalSprint = 0.14f;
	float walkPulseInterval = 0.42f;
	float sprintPulseInterval = 0.29f;
	float walkNoiseRadius = 140.0f;
	float sprintNoiseRadius = 320.0f;
	float pulseDuration = 0.55f;
};

struct PlayerConfig
{
	float walkSpeed = 130.0f;
	float sprintSpeed = 300.0f;
	float speedRampUpPerSec = 260.0f;
	float speedRampDownPerSec = 320.0f;

	float hitboxHalfW = 48.0f;
	float hitboxHalfH = 48.0f;
	float turnRateDegPerSec = 220.0f;
	float spriteFacingOffsetDeg = 90.0f;
	float aimSnapDegrees = 1.25f;

	float bodyTintR = 0.24f;
	float bodyTintG = 0.14f;
	float bodyTintB = 0.085f;
	float desiredSpriteBoxSize = 96.0f;

	FlashlightConfig flashlight;

	FlashlightMeterConfig flashlightMeter;
	FlashlightStunConfig flashlightStun;

	SanityMeterConfig sanityMeter;
	HealthMeterConfig healthMeter;

	FootstepConfig footstep;
};
