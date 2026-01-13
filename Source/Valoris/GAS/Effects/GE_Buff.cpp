// Copyright Valoris. All Rights Reserved.

#include "GE_Buff.h"

UGE_Buff::UGE_Buff()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// 默认时长 5 秒，可在蓝图中覆盖
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.0f));

	// GrantedTags (如 State.Buffed) 在蓝图子类中通过 Components 配置
	// 不在 C++ 构造函数中调用 FindOrAddComponent，会导致崩溃
}
