// Copyright Valoris. All Rights Reserved.

#include "GE_Cooldown_Attack.h"
#include "ValorisGameplayTags.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UGE_Cooldown_Attack::UGE_Cooldown_Attack()
{
	DurationPolicy = EGameplayEffectDurationType::HasDuration;

	// 默认冷却时间 1 秒（实际会根据攻速动态计算）
	FScalableFloat Duration;
	Duration.Value = 1.0f;
	DurationMagnitude = FGameplayEffectModifierMagnitude(Duration);
}

void UGE_Cooldown_Attack::PostCDOContruct()
{
	Super::PostCDOContruct();

	// 添加冷却 Tag 组件
	UTargetTagsGameplayEffectComponent& TargetTagsComponent = FindOrAddComponent<UTargetTagsGameplayEffectComponent>();
	FInheritedTagContainer TagContainer;
	TagContainer.AddTag(FValorisGameplayTags::Get().Cooldown_Attack);
	TargetTagsComponent.SetAndApplyTargetTagChanges(TagContainer);
}
