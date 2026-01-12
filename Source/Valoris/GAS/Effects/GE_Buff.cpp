// Copyright Valoris. All Rights Reserved.

#include "GE_Buff.h"
#include "../ValorisGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UGE_Buff::UGE_Buff()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// 默认时长 5 秒，可在蓝图中覆盖
	DurationMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(5.0f));

	// 应用时添加 Buffed 状态标签（使用 UE 5.7 新 API）
	UTargetTagsGameplayEffectComponent& TargetTagsComponent = FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
	FInheritedTagContainer TagContainer;
	TagContainer.AddTag(FValorisGameplayTags::State_Buffed);
	TargetTagsComponent.SetAndApplyTargetTagChanges(TagContainer);
}
