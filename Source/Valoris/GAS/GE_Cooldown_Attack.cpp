// Copyright Valoris. All Rights Reserved.

#include "GE_Cooldown_Attack.h"
#include "ValorisGameplayTags.h"

UGE_Cooldown_Attack::UGE_Cooldown_Attack()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// 默认冷却时间 1 秒（实际会根据攻速动态计算）
	FScalableFloat Duration;
	Duration.Value = 1.0f;
	DurationMagnitude = FGameplayEffectModifierMagnitude(Duration);

	// 冷却 Tag 需要在蓝图中配置：
	// 打开蓝图 -> Components -> 添加 TargetTagsGameplayEffectComponent
	// 设置 Add Tags -> Cooldown.Attack
}
