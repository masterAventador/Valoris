// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * Valoris 游戏中使用的所有 GameplayTag
 * 统一在此声明，避免硬编码字符串
 * 直接通过 FValorisGameplayTags::TagName 访问
 */
struct FValorisGameplayTags
{
	static void InitializeNativeTags();

	// ========== 事件标签 ==========
	static FGameplayTag Event_Attack;			// Event.Attack - 攻击事件
	static FGameplayTag Event_Attack_Hit;		// Event.Attack.Hit - 攻击命中事件（AnimNotify 发送）

	// ========== 数据标签 ==========
	static FGameplayTag Data_Damage;			// Data.Damage - 伤害值（SetByCaller）

	// ========== 状态标签 ==========
	static FGameplayTag State_Dead;				// State.Dead - 死亡状态
	static FGameplayTag State_Stunned;			// State.Stunned - 眩晕状态

	// ========== 技能标签 ==========
	static FGameplayTag Ability_Attack;			// Ability.Attack - 攻击技能

	// ========== 冷却标签 ==========
	static FGameplayTag Cooldown_Attack;		// Cooldown.Attack - 攻击冷却
};
