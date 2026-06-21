// Copyright Valoris. All Rights Reserved.
#include "ArenaSpawnUtils.h"

FVector ArenaSpawn::ComputeRingSpawnLocation(const FVector& Center, int32 Index, float Radius, float HeightOffset)
{
    constexpr float GoldenAngleDeg = 137.5f;
    const float AngleDeg = Index * GoldenAngleDeg;
    const FVector Dir = FRotator(0.f, AngleDeg, 0.f).Vector();
    return Center + Dir * Radius + FVector(0.f, 0.f, HeightOffset);
}
