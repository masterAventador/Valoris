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
	static FGameplayTag State_Buffed;			// State.Buffed - 增益状态

	// ========== 技能标签（按类型分类） ==========
	// 格式: Ability.[类型].[角色].[技能名]

	// --- 近战单体 (Melee.Single) ---
	static FGameplayTag Ability_Melee_Single_Aric_Attack;			// Ability.Melee.Single.Aric.Attack - Aric 普攻
	static FGameplayTag Ability_Melee_Single_Aric_ShieldBash;		// Ability.Melee.Single.Aric.ShieldBash - Aric 盾击 (Q)

	// --- 近战范围 (Melee.AOE) ---
	static FGameplayTag Ability_Melee_AOE_Aric_WhirlwindSlash;		// Ability.Melee.AOE.Aric.WhirlwindSlash - Aric 旋风斩 (W)

	// --- 远程单体 (Ranged.Single) ---
	static FGameplayTag Ability_Ranged_Single_Tower_Attack;			// Ability.Ranged.Single.Tower.Attack - 塔攻击

	// --- 位移 (Movement) ---
	static FGameplayTag Ability_Movement_Aric_Charge;				// Ability.Movement.Aric.Charge - Aric 冲锋 (E)

	// --- 增益 (Buff) ---
	static FGameplayTag Ability_Buff_Aric_BattleCry;				// Ability.Buff.Aric.BattleCry - Aric 战吼 (R)

	// ========== 通用技能标签（保留兼容） ==========
	static FGameplayTag Ability_Attack;			// Ability.Attack - 通用攻击技能
};
