// Copyright Valoris. All Rights Reserved.
#include "ArenaCombatUtils.h"

bool ArenaCombat::AdvanceContactDamageTimer(float& Accumulator, float DeltaTime, float Interval)
{
    if (Interval <= 0.f)
    {
        return false;
    }
    Accumulator += DeltaTime;
    if (Accumulator >= Interval)
    {
        Accumulator -= Interval;
        return true;
    }
    return false;
}

float ArenaCombat::ComputeMitigatedDamage(float BaseDamage, float Defense)
{
    return FMath::Max(1.f, BaseDamage - Defense);
}

ArenaCombat::FEnemyAttackStep ArenaCombat::StepEnemyAttack(
    EEnemyAttackPhase Current, float& Accumulator, float DeltaTime,
    float WindupDuration, float RecoveryDuration, bool bInAttackRange)
{
    switch (Current)
    {
    case EEnemyAttackPhase::Approaching:
        if (bInAttackRange)
        {
            Accumulator = 0.f;
            return { EEnemyAttackPhase::WindingUp, false };
        }
        return { EEnemyAttackPhase::Approaching, false };

    case EEnemyAttackPhase::WindingUp:
        if (!bInAttackRange)
        {
            Accumulator = 0.f;   // 玩家走出距离，前摇被打断
            return { EEnemyAttackPhase::Approaching, false };
        }
        Accumulator += DeltaTime;
        if (Accumulator >= WindupDuration)
        {
            Accumulator = 0.f;
            return { EEnemyAttackPhase::Recovering, true };   // 挥击
        }
        return { EEnemyAttackPhase::WindingUp, false };

    case EEnemyAttackPhase::Recovering:
        Accumulator += DeltaTime;
        if (Accumulator >= RecoveryDuration)
        {
            Accumulator = 0.f;
            return { EEnemyAttackPhase::Approaching, false };
        }
        return { EEnemyAttackPhase::Recovering, false };
    }
    return { EEnemyAttackPhase::Approaching, false };
}

bool ArenaCombat::ShouldEnemyStrikeConnect(float DistanceToTarget, float AttackReach, bool bTargetInvincible)
{
    return !bTargetInvincible && DistanceToTarget <= AttackReach;
}
