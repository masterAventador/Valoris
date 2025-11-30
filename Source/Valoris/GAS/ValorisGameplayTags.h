// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Valoris 游戏中使用的所有 GameplayTag
 * 统一在此声明，避免硬编码字符串
 */
struct FValorisGameplayTags
{
public:
	static const FValorisGameplayTags& Get() { return GameplayTags; }

	static void InitializeNativeTags();

	// ========== 事件标签 ==========
	FGameplayTag Event_Attack;			// Event.Attack - 攻击事件

	// ========== 数据标签 ==========
	FGameplayTag Data_Damage;			// Data.Damage - 伤害值（SetByCaller）

	// ========== 状态标签 ==========
	FGameplayTag State_Dead;			// State.Dead - 死亡状态
	FGameplayTag State_Stunned;			// State.Stunned - 眩晕状态

	// ========== 技能标签 ==========
	FGameplayTag Ability_Attack;		// Ability.Attack - 攻击技能

	// ========== 冷却标签 ==========
	FGameplayTag Cooldown_Attack;		// Cooldown.Attack - 攻击冷却

private:
	static FValorisGameplayTags GameplayTags;
};
