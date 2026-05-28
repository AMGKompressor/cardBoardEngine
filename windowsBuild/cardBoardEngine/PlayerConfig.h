// cardBoard — tunable player stats (like a Java config / settings class)
#pragma once

namespace CardBoard
{
	struct FlashlightConfig
	{
		float halfAngleDeg = 22.0f;
		float beamRange = 460.0f;
		float featherDeg = 4.0f;
		float maskAlpha = 0.98f;
		float ambientRadius = 72.0f;
		float ambientFeather = 14.0f;
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
		FootstepConfig footstep;
	};
}
