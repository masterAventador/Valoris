// Copyright Valoris. All Rights Reserved.

#include "ValorisGameplayTags.h"
#include "GameplayTagsManager.h"

FValorisGameplayTags FValorisGameplayTags::GameplayTags;

void FValorisGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	// 事件标签
	GameplayTags.Event_Attack = Manager.AddNativeGameplayTag(
		FName("Event.Attack"),
		FString("Attack event triggered when performing an attack")
	);

	// 数据标签
	GameplayTags.Data_Damage = Manager.AddNativeGameplayTag(
		FName("Data.Damage"),
		FString("Damage value for SetByCaller")
	);

	// 状态标签
	GameplayTags.State_Dead = Manager.AddNativeGameplayTag(
		FName("State.Dead"),
		FString("Character is dead")
	);

	GameplayTags.State_Stunned = Manager.AddNativeGameplayTag(
		FName("State.Stunned"),
		FString("Character is stunned")
	);

	// 技能标签
	GameplayTags.Ability_Attack = Manager.AddNativeGameplayTag(
		FName("Ability.Attack"),
		FString("Basic attack ability")
	);

	// 冷却标签
	GameplayTags.Cooldown_Attack = Manager.AddNativeGameplayTag(
		FName("Cooldown.Attack"),
		FString("Attack ability cooldown")
	);
}
