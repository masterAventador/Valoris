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
