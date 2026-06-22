// Copyright Valoris. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "../Core/ArenaSpawnUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArenaSpawnRingLocationTest,
    "Valoris.ArenaSpawn.ComputeRingSpawnLocation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FArenaSpawnRingLocationTest::RunTest(const FString& Parameters)
{
    const FVector Center(1000.f, -500.f, 200.f);
    const float Radius = 1200.f;
    const float Height = 100.f;

    const FVector P0 = ArenaSpawn::ComputeRingSpawnLocation(Center, 0, Radius, Height);
    TestEqual(TEXT("Index0 水平距离应等于半径"), FVector::Dist2D(P0, Center), (double)Radius, 0.1);
    TestEqual(TEXT("Index0 Z 应为 Center.Z+Height"), (double)P0.Z, (double)(Center.Z + Height), 0.1);

    const FVector P1 = ArenaSpawn::ComputeRingSpawnLocation(Center, 1, Radius, Height);
    TestEqual(TEXT("Index1 水平距离应等于半径"), FVector::Dist2D(P1, Center), (double)Radius, 0.1);
    TestTrue(TEXT("不同 Index 方向不同（不扎堆）"), !P0.Equals(P1, 1.f));

    return true;
}

#endif
