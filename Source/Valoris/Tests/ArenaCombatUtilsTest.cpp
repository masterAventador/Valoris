// Copyright Valoris. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "../Combat/ArenaCombatUtils.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArenaComputeMitigatedDamageTest,
    "Valoris.ArenaCombat.ComputeMitigatedDamage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FArenaComputeMitigatedDamageTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("100 伤害 - 5 防御 = 95"), ArenaCombat::ComputeMitigatedDamage(100.f, 5.f), 95.f, 0.001f);
    TestEqual(TEXT("3 伤害 - 5 防御 = 最低 1"), ArenaCombat::ComputeMitigatedDamage(3.f, 5.f), 1.f, 0.001f);
    TestEqual(TEXT("等量：5 - 5 = 最低 1"), ArenaCombat::ComputeMitigatedDamage(5.f, 5.f), 1.f, 0.001f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArenaStepEnemyAttackTest,
    "Valoris.ArenaCombat.StepEnemyAttack",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FArenaStepEnemyAttackTest::RunTest(const FString& Parameters)
{
    using namespace ArenaCombat;
    float Acc = 0.f;

    // 离玩家远：停在 Approaching，不挥击
    FEnemyAttackStep S = StepEnemyAttack(EEnemyAttackPhase::Approaching, Acc, 0.1f, 0.4f, 0.5f, false);
    TestTrue(TEXT("远→仍 Approaching"), S.Phase == EEnemyAttackPhase::Approaching);
    TestFalse(TEXT("远→不挥击"), S.bStrike);

    // 进入攻击距离：转 WindingUp，累加器清零，不挥击
    Acc = 3.f;
    S = StepEnemyAttack(EEnemyAttackPhase::Approaching, Acc, 0.1f, 0.4f, 0.5f, true);
    TestTrue(TEXT("进距离→WindingUp"), S.Phase == EEnemyAttackPhase::WindingUp);
    TestEqual(TEXT("进 WindingUp 清零累加器"), Acc, 0.f, 0.001f);
    TestFalse(TEXT("刚进 WindingUp 不挥击"), S.bStrike);

    // WindingUp 累加未到：保持 WindingUp，不挥击
    Acc = 0.f;
    S = StepEnemyAttack(EEnemyAttackPhase::WindingUp, Acc, 0.2f, 0.4f, 0.5f, true);
    TestTrue(TEXT("前摇未满→仍 WindingUp"), S.Phase == EEnemyAttackPhase::WindingUp);
    TestFalse(TEXT("前摇未满不挥击"), S.bStrike);

    // WindingUp 累加达 windup：转 Recovering 并挥击，累加器清零
    S = StepEnemyAttack(EEnemyAttackPhase::WindingUp, Acc, 0.3f, 0.4f, 0.5f, true);
    TestTrue(TEXT("前摇满→Recovering"), S.Phase == EEnemyAttackPhase::Recovering);
    TestTrue(TEXT("前摇满→挥击"), S.bStrike);
    TestEqual(TEXT("挥击后清零累加器"), Acc, 0.f, 0.001f);

    // WindingUp 中玩家跑出距离：前摇被打断，回 Approaching，不挥击
    Acc = 0.3f;
    S = StepEnemyAttack(EEnemyAttackPhase::WindingUp, Acc, 0.1f, 0.4f, 0.5f, false);
    TestTrue(TEXT("跑出距离→打断回 Approaching"), S.Phase == EEnemyAttackPhase::Approaching);
    TestFalse(TEXT("打断不挥击"), S.bStrike);
    TestEqual(TEXT("打断清零累加器"), Acc, 0.f, 0.001f);

    // Recovering 累加达 recovery：回 Approaching
    Acc = 0.f;
    S = StepEnemyAttack(EEnemyAttackPhase::Recovering, Acc, 0.6f, 0.4f, 0.5f, true);
    TestTrue(TEXT("冷却满→回 Approaching"), S.Phase == EEnemyAttackPhase::Approaching);
    TestFalse(TEXT("冷却满不挥击"), S.bStrike);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FArenaShouldStrikeConnectTest,
    "Valoris.ArenaCombat.ShouldEnemyStrikeConnect",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FArenaShouldStrikeConnectTest::RunTest(const FString& Parameters)
{
    using namespace ArenaCombat;
    TestTrue(TEXT("范围内非无敌→命中"), ShouldEnemyStrikeConnect(100.f, 150.f, false));
    TestFalse(TEXT("范围外→挥空"), ShouldEnemyStrikeConnect(200.f, 150.f, false));
    TestFalse(TEXT("范围内但无敌→挥空"), ShouldEnemyStrikeConnect(100.f, 150.f, true));
    return true;
}

#endif
