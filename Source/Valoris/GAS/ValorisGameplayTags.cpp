// Copyright Valoris. All Rights Reserved.

#include "ValorisGameplayTags.h"
#include "GameplayTagsManager.h"

// 静态成员定义
FGameplayTag FValorisGameplayTags::Event_Attack;
FGameplayTag FValorisGameplayTags::Event_Attack_Hit;
FGameplayTag FValorisGameplayTags::Data_Damage;
FGameplayTag FValorisGameplayTags::State_Dead;
FGameplayTag FValorisGameplayTags::State_Stunned;
FGameplayTag FValorisGameplayTags::State_Buffed;
FGameplayTag FValorisGameplayTags::State_Invincible;
FGameplayTag FValorisGameplayTags::Ability_Attack;
// 近战单体
FGameplayTag FValorisGameplayTags::Ability_Melee_Single_Aric_Attack;
FGameplayTag FValorisGameplayTags::Ability_Melee_Single_Aric_ShieldBash;
// 近战范围
FGameplayTag FValorisGameplayTags::Ability_Melee_AOE_Aric_WhirlwindSlash;
// 远程单体
FGameplayTag FValorisGameplayTags::Ability_Ranged_Single_Tower_Attack;
// 位移
FGameplayTag FValorisGameplayTags::Ability_Movement_Aric_Dodge;
// 增益
FGameplayTag FValorisGameplayTags::Ability_Buff_Aric_BattleCry;

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

	State_Buffed = Manager.AddNativeGameplayTag(
		FName("State.Buffed"),
		FString("Character has active buff")
	);

	State_Invincible = Manager.AddNativeGameplayTag(
		FName("State.Invincible"),
		FString("Character is invincible (dodge i-frames)")
	);

	// 通用技能标签
	Ability_Attack = Manager.AddNativeGameplayTag(
		FName("Ability.Attack"),
		FString("Basic attack ability")
	);

	// ========== 技能标签（按类型分类） ==========
	// 格式: Ability.[类型].[角色].[技能名]

	// --- 近战单体 (Melee.Single) ---
	Ability_Melee_Single_Aric_Attack = Manager.AddNativeGameplayTag(
		FName("Ability.Melee.Single.Aric.Attack"),
		FString("Aric basic melee attack")
	);

	Ability_Melee_Single_Aric_ShieldBash = Manager.AddNativeGameplayTag(
		FName("Ability.Melee.Single.Aric.ShieldBash"),
		FString("Aric shield bash ability (Q) - melee single with stun")
	);

	// --- 近战范围 (Melee.AOE) ---
	Ability_Melee_AOE_Aric_WhirlwindSlash = Manager.AddNativeGameplayTag(
		FName("Ability.Melee.AOE.Aric.WhirlwindSlash"),
		FString("Aric whirlwind slash ability (W) - melee AOE")
	);

	// --- 远程单体 (Ranged.Single) ---
	Ability_Ranged_Single_Tower_Attack = Manager.AddNativeGameplayTag(
		FName("Ability.Ranged.Single.Tower.Attack"),
		FString("Tower ranged single target attack")
	);

	// --- 位移 (Movement) ---
	Ability_Movement_Aric_Dodge = Manager.AddNativeGameplayTag(
		FName("Ability.Movement.Aric.Dodge"),
		FString("Aric dodge roll ability (Space) - movement with i-frames")
	);

	// --- 增益 (Buff) ---
	Ability_Buff_Aric_BattleCry = Manager.AddNativeGameplayTag(
		FName("Ability.Buff.Aric.BattleCry"),
		FString("Aric battle cry ability (R) - self buff")
	);
}
