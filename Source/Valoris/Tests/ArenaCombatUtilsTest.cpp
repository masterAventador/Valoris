// Copyright Valoris. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "../Combat/ArenaCombatUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArenaContactDamageTimerTest,
    "Valoris.ArenaCombat.AdvanceContactDamageTimer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FArenaContactDamageTimerTest::RunTest(const FString& Parameters)
{
    float Acc = 0.f;
    TestFalse(TEXT("0.3 < 0.5 不触发"), ArenaCombat::AdvanceContactDamageTimer(Acc, 0.3f, 0.5f));
    TestTrue(TEXT("累计到 0.6 触发"), ArenaCombat::AdvanceContactDamageTimer(Acc, 0.3f, 0.5f));
    TestEqual(TEXT("触发后余量约 0.1"), Acc, 0.1f, 0.001f);
    float Acc2 = 10.f;
    TestFalse(TEXT("Interval<=0 不触发"), ArenaCombat::AdvanceContactDamageTimer(Acc2, 1.f, 0.f));
    return true;
}

#endif
