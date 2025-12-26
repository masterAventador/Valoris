// Copyright Valoris. All Rights Reserved.

#include "ValorisGameplayTags.h"
#include "GameplayTagsManager.h"

// 静态成员定义
FGameplayTag FValorisGameplayTags::Event_Attack;
FGameplayTag FValorisGameplayTags::Event_Attack_Hit;
FGameplayTag FValorisGameplayTags::Data_Damage;
FGameplayTag FValorisGameplayTags::State_Dead;
FGameplayTag FValorisGameplayTags::State_Stunned;
FGameplayTag FValorisGameplayTags::Ability_Attack;
FGameplayTag FValorisGameplayTags::Cooldown_Attack;

void FValorisGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	// 事件标签
	Event_Attack = Manager.AddNativeGameplayTag(
		FName("Event.Attack"),
		FString("Attack event triggered when performing an attack")
	);

	Event_Attack_Hit = Manager.AddNativeGameplayTag(
		FName("Event.Attack.Hit"),
		FString("Attack hit event sent by AnimNotify during attack montage")
	);

	// 数据标签
	Data_Damage = Manager.AddNativeGameplayTag(
		FName("Data.Damage"),
		FString("Damage value for SetByCaller")
	);

	// 状态标签
	State_Dead = Manager.AddNativeGameplayTag(
		FName("State.Dead"),
		FString("Character is dead")
	);

	State_Stunned = Manager.AddNativeGameplayTag(
		FName("State.Stunned"),
		FString("Character is stunned")
	);

	// 技能标签
	Ability_Attack = Manager.AddNativeGameplayTag(
		FName("Ability.Attack"),
		FString("Basic attack ability")
	);

	// 冷却标签
	Cooldown_Attack = Manager.AddNativeGameplayTag(
		FName("Cooldown.Attack"),
		FString("Attack ability cooldown")
	);
}
