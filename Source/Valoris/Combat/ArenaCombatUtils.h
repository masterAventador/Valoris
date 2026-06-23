// Copyright Valoris. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

namespace ArenaCombat
{
    // 接触伤害节拍：累计 DeltaTime，达到 Interval 即返回 true 并扣掉一个 Interval。
    // 返回 true = 本次 tick 应施加一次接触伤害。Interval<=0 视为禁用，恒返回 false。
    VALORIS_API bool AdvanceContactDamageTimer(float& Accumulator, float DeltaTime, float Interval);

    // 伤害减伤的唯一收口：基础伤害减去防御，最低为 1。
    VALORIS_API float ComputeMitigatedDamage(float BaseDamage, float Defense);

    // 敌人攻击相位
    enum class EEnemyAttackPhase : uint8
    {
        Approaching,   // 追玩家
        WindingUp,     // 前摇蓄力
        Recovering     // 挥击后冷却
    };

    // 推进一帧攻击相位的结果
    struct FEnemyAttackStep
    {
        EEnemyAttackPhase Phase;   // 下一相位
        bool bStrike;              // 本帧是否触发挥击
    };

    // 推进一帧攻击状态机。Accumulator 为相位内累计时间（按引用修改）。
    VALORIS_API FEnemyAttackStep StepEnemyAttack(
        EEnemyAttackPhase Current, float& Accumulator, float DeltaTime,
        float WindupDuration, float RecoveryDuration, bool bInAttackRange);

    // 挥击是否结算伤害：目标在攻击触及范围内且未带无敌。
    VALORIS_API bool ShouldEnemyStrikeConnect(float DistanceToTarget, float AttackReach, bool bTargetInvincible);
}
