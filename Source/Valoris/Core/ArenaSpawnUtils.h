// Copyright Valoris. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

namespace ArenaSpawn
{
    // 在以 Center 为圆心、半径 Radius 的水平圆环上，按 Index 计算生成点。
    // 角度用黄金角(137.5°)分布，使连续生成的敌人均匀散布玩家四周、不扎堆同方向。
    // HeightOffset 抬高 Z，避免卡进地面。
    VALORIS_API FVector ComputeRingSpawnLocation(const FVector& Center, int32 Index, float Radius, float HeightOffset);
}
