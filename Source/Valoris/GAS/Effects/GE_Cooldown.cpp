// Copyright Valoris. All Rights Reserved.

#include "GE_Cooldown.h"

UGE_Cooldown::UGE_Cooldown()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// 默认时长 1 秒，实际值由 ApplyCooldown 动态设置
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(1.0f));
}
