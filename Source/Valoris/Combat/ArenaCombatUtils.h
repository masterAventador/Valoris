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
}
