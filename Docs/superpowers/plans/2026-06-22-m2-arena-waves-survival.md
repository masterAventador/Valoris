# M2 竞技场波次生存 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 M1 的「砍会追你的假人」雏形，补成「一波波环形涌来、被围会掉血、血空失败 / 撑过 N 波胜利、结算屏一键重开」的完整可玩循环骨架。

**Architecture:** 最大化复用现有塔防 GameMode 的波次基础设施（`UWaveData` + `StartWaves/StartNextWave/SpawnNextEnemy/CheckWaveCompletion` + 事件），只改三处：①敌人生成位置从 Spline 路径起点 → 玩家四周环形；②胜负模型从「基地血量」→「玩家死亡(失败) / 撑过 N 波(胜利)」；③接 HUD + 结算屏。敌人贴身接触按间隔对玩家施加 `GE_Damage`（复用现有伤害管线）。纯逻辑（环形坐标、接触节拍）抽成自由函数走 TDD + UE Automation 测试；GAS/GameMode/UMG 集成走「编译 + MCP/PIE 亲眼验证」。

**Tech Stack:** UE 5.8、C++、GAS（GameplayEffect/AttributeSet/ASC）、UMG、UE Automation 测试、Unreal MCP（spawn/PIE/读 ASC/读 log/跑测试）。

## Global Constraints

- **引擎/分支**：UE 5.8；在 `arena-m2` 分支开发（已建，当前分支）。不在 master 改。
- **编译命令**：`& "G:\Unreal Engines\UE_5.8\Engine\Build\BatchFiles\Build.bat" ValorisEditor Win64 Development -project="F:\Valoris\Valoris.uproject" -waitmutex`，预期结尾 `Result: Succeeded`。
- **Live Coding 限制**：新增 `UPROPERTY`/成员函数/类等反射改动，Live Coding 编不进，必须关编辑器用上面命令完整重编再开编辑器。纯函数体改动可 Live Coding 热编。
- **验证哲学（UE 现实，spec §8 已定）**：纯逻辑自由函数（环形坐标、接触节拍）必须 TDD（先写失败的 Automation 测试→实现→测试通过）；GAS/Actor/UMG/相机这类集成胶水代码无法脱离运行环境单测，用「编译通过 + MCP 读运行时状态 / PIE 亲眼看」验证，并在该 Task 写明预期观察值。**不允许给集成代码编造假单测冒充 TDD。**
- **伤害模型**：敌人贴身接触掉血（survivors-like 品类标准），不是临时方案；敌人主动挥击/前摇攻击留 M3。
- **波数**：M2 先 3 波（数据驱动，`DA_ArenaWaves` 资产），M5 平衡再扩 8~10 波。
- **M2 不做**：敌人主动攻击/闪避/多敌种(M3)、三选一升级 UI(M4)、玩家受击动画/击杀数计时/数值平衡(M3/M5)。
- **删除边界**：M2 只删被直接取代的死代码（基地血量整套、M1 临时生成）；塔/Spline/经济(ResourceManager)/RTS 输入留给独立「塔系统清理」里程碑。
- **类引用约定**：`PlayerControllerClass`/`DefaultPawnClass`/资产引用一律指 BP 版（M1 教训）。
- **提交规范**：每个 Task 末尾提交；commit message 用中文（conventional 前缀可英文）；不加任何 AI 署名；`git add` 指定文件，不用 `-A`。
- **MCP 调用语法**：`call_tool(tool_name=..., toolset_name=..., arguments={...})`；启动顺序＝先开 UE 编辑器（server 监听 127.0.0.1:8000）再连 Claude Code。
- **MCP 验证常用工具**（实现者验证步骤用）：
  - 跑 Automation 测试：`AutomationTestToolset.AutomationTestToolset` 的 `DiscoverTests` → `RunTests {"TestNames":[...]}` → `GetTestResults`。
  - 开/关 PIE：`EditorToolset.EditorAppToolset` 的 `StartPIE {"options":{"bSimulate":false,"playMode":"PlayMode_InViewPort","warmupSeconds":1}}` / `StopPIE` / `IsPIERunning`。
  - 读运行时属性：`GASToolsets.AbilitySystemInspectorToolset.GetAttributeValues {"actor":{"refPath":"<玩家Pawn>"}}` → 取 `attributeName=="Health"` 的 `currentValue`。
  - 读关卡 / 找 actor：`editor_toolset.toolsets.scene.SceneTools`；看视口（可按类标注）：`EditorAppToolset.CaptureViewport`；读日志：`EditorToolset.LogsToolset`。

## 现有可复用资产 / 关键符号（实现者必读，避免重复造轮子）

- `Source/Valoris/Data/WaveData.h`：`UWaveData{ TArray<FWaveInfo> Waves; GetWaveCount(); GetWaveInfo(i); }`，`FWaveInfo{ FText WaveName; TArray<FEnemySpawnInfo> Enemies; float DelayAfterWave; int32 GoldReward; }`，`FEnemySpawnInfo{ TSubclassOf<AEnemyBase> EnemyClass; int32 Count; float SpawnInterval; }`。
- `Source/Valoris/Core/ValorisGameMode.{h,cpp}`：波次流程 + 事件 `OnWaveStarted(int32)` / `OnWaveCompleted(int32)` / `OnAllWavesCompleted()` / `OnGameOver(bool bVictory)`；`AliveEnemyCount`（protected）；胜利已在 `CheckWaveCompletion()` 广播 `OnGameOver(true)`。
- `Source/Valoris/GAS/ValorisAttributeSet.cpp::PostGameplayEffectExecute`：伤害结算 + 死亡判定（血空时 `Cast<AEnemyBase>` → `MarkAsKilled()+Destroy()`）。**玩家死亡镜像点就在此**。
- 伤害施加管线（见 `GA_MeleeAttack::ApplyDamageToTarget`）：`SourceASC->MakeOutgoingSpec(GE_Damage,1,Ctx)` → `Spec.Data->SetSetByCallerMagnitude(FValorisGameplayTags::Data_Damage, Dmg)` → `TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data)`。`GE_Damage` 在 `Source/Valoris/GAS/GE_Damage.h`。
- `Source/Valoris/GAS/ValorisGameplayTags.h`：`Data_Damage`（SetByCaller）、`Event_Attack_Hit`、`Ability_Melee_Single_Aric_Attack` 均已定义。
- `Source/Valoris/Enemy/EnemyBase.cpp::Tick`：已实现追玩家、`Distance > 120` 才移动（≤120 算贴身）。接触伤害加在这里。
- `Source/Valoris/Character/AricHero.h`：玩家英雄，继承 `AValorisCharacterBase`（自带 `AbilitySystemComponent`/`AttributeSet`/`GetAbilitySystemComponent()`）。
- `Source/Valoris/UI/ValorisHUD.{h,cpp}`：`UUserWidget`，用 `meta=(BindWidget)` 绑控件，已绑 `OnWaveStarted`/`OnGameOver`，有 `ShowGameResult(bVictory)`。

## File Structure

| 文件 | 职责 | 动作 |
|------|------|------|
| `Source/Valoris/Core/ArenaSpawnUtils.{h,cpp}` | 环形生成坐标（纯函数） | 新建 |
| `Source/Valoris/Combat/ArenaCombatUtils.{h,cpp}` | 接触伤害节拍（纯函数） | 新建 |
| `Source/Valoris/Tests/ArenaSpawnUtilsTest.cpp` | 环形坐标 Automation 测试 | 新建 |
| `Source/Valoris/Tests/ArenaCombatUtilsTest.cpp` | 接触节拍 Automation 测试 | 新建 |
| `Source/Valoris/Core/ValorisGameMode.{h,cpp}` | 环形波次、删基地血量/M1临时、加剩余敌人事件、玩家死亡入口 | 改 |
| `Source/Valoris/Enemy/EnemyBase.{h,cpp}` | 贴身接触伤害；修 `OnReachedEnd`（去 DamageBase） | 改 |
| `Source/Valoris/GAS/ValorisAttributeSet.cpp` | 玩家血空→通知 GameMode 失败 | 改 |
| `Source/Valoris/UI/ValorisHUD.{h,cpp}` | 玩家血条/剩余敌人/波次横幅；移除金币/基地血量；结算屏+重开 | 改 |
| `Content/Data/DA_ArenaWaves.uasset` | 3 波递增配置（编辑器/MCP 资产） | 新建 |
| `BP_ArenaGameMode` / `WBP_ValorisHUD`(或现有) | 指配 WaveData、绑新 HUD 控件、结算面板 | 编辑器配置 |

---

## 任务 1：环形生成坐标（纯函数，TDD）

**Files:**
- Create: `Source/Valoris/Core/ArenaSpawnUtils.h`
- Create: `Source/Valoris/Core/ArenaSpawnUtils.cpp`
- Test: `Source/Valoris/Tests/ArenaSpawnUtilsTest.cpp`

**Interfaces:**
- Produces: `ArenaSpawn::ComputeRingSpawnLocation(const FVector& Center, int32 Index, float Radius, float HeightOffset) -> FVector`（任务 2 在 `SpawnEnemy` 调用）。

- [ ] **Step 1: 写失败的测试**

`Source/Valoris/Tests/ArenaSpawnUtilsTest.cpp`：
```cpp
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
    TestEqual(TEXT("Index0 水平距离应等于半径"), FVector::Dist2D(P0, Center), Radius, 0.1f);
    TestEqual(TEXT("Index0 Z 应为 Center.Z+Height"), P0.Z, Center.Z + Height, 0.1f);

    const FVector P1 = ArenaSpawn::ComputeRingSpawnLocation(Center, 1, Radius, Height);
    TestEqual(TEXT("Index1 水平距离应等于半径"), FVector::Dist2D(P1, Center), Radius, 0.1f);
    TestTrue(TEXT("不同 Index 方向不同（不扎堆）"), !P0.Equals(P1, 1.f));

    return true;
}

#endif
```

- [ ] **Step 2: 建头文件 + 占位实现（先让它编译但返回错值，制造 RED）**

`Source/Valoris/Core/ArenaSpawnUtils.h`：
```cpp
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
```
`Source/Valoris/Core/ArenaSpawnUtils.cpp`（占位返回 ZeroVector）：
```cpp
// Copyright Valoris. All Rights Reserved.
#include "ArenaSpawnUtils.h"

FVector ArenaSpawn::ComputeRingSpawnLocation(const FVector& Center, int32 Index, float Radius, float HeightOffset)
{
    return FVector::ZeroVector; // 占位：任务此步应让测试 FAIL
}
```

- [ ] **Step 3: 编译**

Run: `& "G:\Unreal Engines\UE_5.8\Engine\Build\BatchFiles\Build.bat" ValorisEditor Win64 Development -project="F:\Valoris\Valoris.uproject" -waitmutex`
Expected: `Result: Succeeded`

- [ ] **Step 4: 跑测试，确认 FAIL（RED）**

开 UE 编辑器后，用 MCP：
```
call_tool(toolset_name="AutomationTestToolset.AutomationTestToolset", tool_name="DiscoverTests", arguments={})
call_tool(toolset_name="AutomationTestToolset.AutomationTestToolset", tool_name="RunTests", arguments={"TestNames":["Valoris.ArenaSpawn.ComputeRingSpawnLocation"]})
call_tool(toolset_name="AutomationTestToolset.AutomationTestToolset", tool_name="GetTestResults", arguments={})
```
Expected: FAIL（`水平距离应等于半径` 断言失败，实际 0）

- [ ] **Step 5: 写真实现**

`ArenaSpawnUtils.cpp` 改为：
```cpp
// Copyright Valoris. All Rights Reserved.
#include "ArenaSpawnUtils.h"

FVector ArenaSpawn::ComputeRingSpawnLocation(const FVector& Center, int32 Index, float Radius, float HeightOffset)
{
    constexpr float GoldenAngleDeg = 137.5f;
    const float AngleDeg = Index * GoldenAngleDeg;
    const FVector Dir = FRotator(0.f, AngleDeg, 0.f).Vector();
    return Center + Dir * Radius + FVector(0.f, 0.f, HeightOffset);
}
```

- [ ] **Step 6: 编译**

Run: 同 Step 3。Expected: `Result: Succeeded`

- [ ] **Step 7: 跑测试，确认 PASS（GREEN）**

MCP 同 Step 4 的 RunTests + GetTestResults。Expected: PASS。

- [ ] **Step 8: 提交**
```bash
git add Source/Valoris/Core/ArenaSpawnUtils.h Source/Valoris/Core/ArenaSpawnUtils.cpp Source/Valoris/Tests/ArenaSpawnUtilsTest.cpp
git commit -m "feat(arena): 环形生成坐标纯函数 + Automation 测试"
```

---

## 任务 2：GameMode 改造为环形波次（接回 StartWaves + 删基地血量/M1临时）

**Files:**
- Modify: `Source/Valoris/Core/ValorisGameMode.h`
- Modify: `Source/Valoris/Core/ValorisGameMode.cpp`
- Modify: `Source/Valoris/Enemy/EnemyBase.cpp`（修 `OnReachedEnd_Implementation` 去掉 `DamageBase` 调用）
- Modify: `Source/Valoris/UI/ValorisHUD.h` / `.cpp`（移除已删的基地血量绑定，保证编译）

**Interfaces:**
- Consumes: `ArenaSpawn::ComputeRingSpawnLocation`（任务 1）。
- Produces: `AValorisGameMode::GetAliveEnemyCount() const -> int32`；事件 `OnEnemyCountChanged(int32 NewCount)`；保留 `OnGameOver(bool)`（任务 4/5/6 消费）；保留 `NotifyPlayerDied()` 的占位（任务 4 实现）。
- Removes: `BaseHealth`/`BaseMaxHealth`/`DamageBase`/`OnBaseHealthChanged`/`GetBaseHealth`/`GetBaseMaxHealth`、`M1TestEnemyClass`/`SpawnM1TestEnemies`。

> 本任务无纯逻辑可单测（全是 GAS/世界集成），按验证哲学走「编译 + MCP/PIE」。

- [ ] **Step 1: 改 `ValorisGameMode.h`**

删除这些成员/声明：`SpawnM1TestEnemies()`、`M1TestEnemyClass`、`DamageBase()`、`GetBaseHealth()`、`GetBaseMaxHealth()`、`OnBaseHealthChanged`、`BaseMaxHealth`、`BaseHealth`、委托声明 `FOnBaseHealthChanged`。

新增（在 Wave 相关区域）：
```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyCountChanged, int32, NewCount);
```
public 区加：
```cpp
// 当前存活敌人数（HUD 显示本波剩余）
UFUNCTION(BlueprintCallable, Category = "Wave")
int32 GetAliveEnemyCount() const { return AliveEnemyCount; }

// 存活敌人数变化事件
UPROPERTY(BlueprintAssignable, Category = "Wave|Events")
FOnEnemyCountChanged OnEnemyCountChanged;

// 玩家死亡 → 失败（由 ValorisAttributeSet 在玩家血量归零时调用；任务 4 实现）
UFUNCTION(BlueprintCallable, Category = "Game")
void NotifyPlayerDied();
```
protected 配置区加（环形半径）：
```cpp
// 竞技场敌人环形生成半径（绕玩家）
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave|Config")
float SpawnRadius = 1200.f;
```
运行时状态区加：
```cpp
// 环形生成累加索引（决定每个敌人在环上的角度）
int32 SpawnIndex = 0;
```

- [ ] **Step 2: 改 `ValorisGameMode.cpp`**

构造函数：保持不变（仍建 `ResourceManager`，经济留待塔清理）。

`BeginPlay` 改为：
```cpp
void AValorisGameMode::BeginPlay()
{
    Super::BeginPlay();

    // 延迟开波：确保玩家 Pawn 已 spawn/possess（环形生成要取玩家位置）
    GetWorld()->GetTimerManager().SetTimer(
        SpawnTimerHandle, this, &AValorisGameMode::StartWaves, 0.5f, false);
}
```

删除整个 `SpawnM1TestEnemies()` 函数。

`StartWaves()` 去掉 EnemyPath 检查：
```cpp
void AValorisGameMode::StartWaves()
{
    if (!WaveData || WaveData->GetWaveCount() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ValorisGameMode: No WaveData configured!"));
        return;
    }

    CurrentWaveIndex = -1;
    SpawnIndex = 0;
    StartNextWave();
}
```

`SpawnEnemy()` 改为环形生成（去掉 EnemyPath / SetPath）：
```cpp
void AValorisGameMode::SpawnEnemy(TSubclassOf<AEnemyBase> EnemyClass)
{
    if (!EnemyClass)
    {
        return;
    }

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    const FVector Center = Player ? Player->GetActorLocation() : FVector::ZeroVector;
    const FVector SpawnLocation = ArenaSpawn::ComputeRingSpawnLocation(Center, SpawnIndex++, SpawnRadius, 100.f);

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEnemyBase* Enemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (Enemy)
    {
        Enemy->OnDestroyed.AddDynamic(this, &AValorisGameMode::OnEnemyDestroyed);
        AliveEnemyCount++;
        OnEnemyCountChanged.Broadcast(AliveEnemyCount);
    }
}
```

`OnEnemyDestroyed()` 末尾在递减后广播数量（保留金币逻辑不动）：
```cpp
void AValorisGameMode::OnEnemyDestroyed(AActor* DestroyedActor)
{
    AliveEnemyCount = FMath::Max(0, AliveEnemyCount - 1);
    OnEnemyCountChanged.Broadcast(AliveEnemyCount);

    if (AEnemyBase* Enemy = Cast<AEnemyBase>(DestroyedActor))
    {
        if (Enemy->WasKilled() && ResourceManager)
        {
            ResourceManager->AddGold(Enemy->GetGoldReward());
        }
    }

    CheckWaveCompletion();
}
```

删除整个 `DamageBase()` 函数。`CheckWaveCompletion()` 保持不变（胜利分支已广播 `OnGameOver(true)`）。

顶部加 include：
```cpp
#include "ArenaSpawnUtils.h"
```
并删除不再需要的 `#include "../Enemy/EnemyPath.h"`（若 SpawnEnemy 不再用 EnemyPath；`EnemyPath` 成员仍在 .h 则保留 include —— 本任务保留 `EnemyPath` 成员字段不动，故 include 保留，仅不再使用）。

> 注：`EnemyPath` 字段及其它塔防路径代码本任务**不删**（留塔清理里程碑），只是不再调用。

- [ ] **Step 3: 修 `EnemyBase.cpp::OnReachedEnd_Implementation`（去掉已删的 DamageBase 调用）**

```cpp
void AEnemyBase::OnReachedEnd_Implementation()
{
    // 竞技场不再有"到达基地"概念（敌人直接追玩家）。路径系统属塔防遗留，留待清理里程碑。
    Destroy();
}
```

- [ ] **Step 4: 修 `ValorisHUD`（移除已删的基地血量引用，保证编译）**

`ValorisHUD.h`：删除 `UpdateBaseHealthDisplay()`、`OnBaseHealthChanged()`、`BaseHealthText` 成员。
`ValorisHUD.cpp`：删除 `InitializeHUD()` 里 `GameMode->OnBaseHealthChanged.AddDynamic(...)` 与 `UpdateBaseHealthDisplay(GameMode->GetBaseHealth(), GameMode->GetBaseMaxHealth());` 两行；删除 `UpdateBaseHealthDisplay()`、`OnBaseHealthChanged()` 两个函数定义。
（金币/波次显示本步保留不动，HUD 完整改造在任务 5。）

- [ ] **Step 5: 编译**

Run: 编译命令（见 Global Constraints）。Expected: `Result: Succeeded`（零错误）。

- [ ] **Step 6: 编辑器配置（用户 / MCP）— 建 3 波数据 + 指配 GameMode**

- 新建 `Content/Data/DA_ArenaWaves`（`UWaveData`）：配 3 个 `FWaveInfo`，每波 `Enemies` 各一组 `BP_EnemyBasic`，数量 5 / 8 / 12，`SpawnInterval` 0.8 / 0.6 / 0.5，`DelayAfterWave` 3。
  - 可用 MCP：`AssetTools` 在 `/Game/Data` 建 `UWaveData` 资产，再用 `ObjectTools.set_properties` 写 `Waves` 数组；或编辑器手建。
- `BP_ArenaGameMode`：把 `WaveData` 指向 `DA_ArenaWaves`；`SpawnRadius` 默认 1200 可调；（`M1TestEnemyClass` 字段已删，无需再清）。

- [ ] **Step 7: MCP / PIE 验证**

用 MCP 观察：
- `EditorAppToolset.StartPIE {"options":{"bSimulate":false,"playMode":"PlayMode_InViewPort","warmupSeconds":1}}` 开 PIE；`SceneTools` 读当前关卡确认在竞技场关卡。
- `EditorAppToolset.CaptureViewport`（`annotations.classFilter` 指 `EnemyBase`）截图确认第 1 波 5 个敌人在玩家四周环形分布、朝玩家移动。
- 预期：开局 0.5s 后第 1 波 5 个环形刷出 → 追向玩家 → 砍光后约 3s 第 2 波（8 个）→ 第 3 波（12 个）→ 砍光最后一波。
- `LogsToolset` 读 log 确认胜利路径输出 `Game Over - Victory!`。

- [ ] **Step 8: 提交**
```bash
git add Source/Valoris/Core/ValorisGameMode.h Source/Valoris/Core/ValorisGameMode.cpp Source/Valoris/Enemy/EnemyBase.cpp Source/Valoris/UI/ValorisHUD.h Source/Valoris/UI/ValorisHUD.cpp
git commit -m "feat(arena): 波次改环形生成接回 StartWaves，删基地血量与 M1 临时生成"
```
（`DA_ArenaWaves.uasset` 等资产改动若由 MCP/编辑器产生，单独 `git add` 对应 .uasset 一并提交。）

---

## 任务 3：敌人贴身接触伤害（纯节拍 TDD + EnemyBase 接入）

**Files:**
- Create: `Source/Valoris/Combat/ArenaCombatUtils.h` / `.cpp`
- Test: `Source/Valoris/Tests/ArenaCombatUtilsTest.cpp`
- Modify: `Source/Valoris/Enemy/EnemyBase.h` / `.cpp`

**Interfaces:**
- Produces: `ArenaCombat::AdvanceContactDamageTimer(float& Accumulator, float DeltaTime, float Interval) -> bool`；`AEnemyBase::ApplyContactDamageTo(AActor* Target)`。

- [ ] **Step 1: 写失败的测试**

`Source/Valoris/Tests/ArenaCombatUtilsTest.cpp`：
```cpp
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
    // 未到间隔，不触发
    TestFalse(TEXT("0.3 < 0.5 不触发"), ArenaCombat::AdvanceContactDamageTimer(Acc, 0.3f, 0.5f));
    // 再加 0.3 达到 0.6 >= 0.5，触发一次并扣 0.5（剩 ~0.1）
    TestTrue(TEXT("累计到 0.6 触发"), ArenaCombat::AdvanceContactDamageTimer(Acc, 0.3f, 0.5f));
    TestEqual(TEXT("触发后余量约 0.1"), Acc, 0.1f, 0.001f);
    // Interval<=0 视为禁用，不触发
    float Acc2 = 10.f;
    TestFalse(TEXT("Interval<=0 不触发"), ArenaCombat::AdvanceContactDamageTimer(Acc2, 1.f, 0.f));
    return true;
}

#endif
```

- [ ] **Step 2: 建头 + 占位实现（RED）**

`Source/Valoris/Combat/ArenaCombatUtils.h`：
```cpp
// Copyright Valoris. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"

namespace ArenaCombat
{
    // 接触伤害节拍：累计 DeltaTime，达到 Interval 即返回 true 并扣掉一个 Interval。
    // 返回 true = 本次 tick 应施加一次接触伤害。Interval<=0 视为禁用，恒返回 false。
    VALORIS_API bool AdvanceContactDamageTimer(float& Accumulator, float DeltaTime, float Interval);
}
```
`Source/Valoris/Combat/ArenaCombatUtils.cpp`（占位）：
```cpp
// Copyright Valoris. All Rights Reserved.
#include "ArenaCombatUtils.h"

bool ArenaCombat::AdvanceContactDamageTimer(float& Accumulator, float DeltaTime, float Interval)
{
    return false; // 占位：此步应让测试 FAIL
}
```

- [ ] **Step 3: 编译**（命令同前）→ `Result: Succeeded`

- [ ] **Step 4: 跑测试确认 FAIL（RED）**

MCP：`RunTests {"TestNames":["Valoris.ArenaCombat.AdvanceContactDamageTimer"]}` → 预期 FAIL（`累计到 0.6 触发` 断言失败）。

- [ ] **Step 5: 写真实现**

`ArenaCombatUtils.cpp`：
```cpp
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
```

- [ ] **Step 6: 编译** → `Result: Succeeded`
- [ ] **Step 7: 跑测试确认 PASS（GREEN）**

- [ ] **Step 8: EnemyBase 接入接触伤害（集成，编译 + PIE 验证）**

`EnemyBase.h` protected 区加：
```cpp
// 接触伤害：贴身时按间隔对玩家施加的 GameplayEffect（默认 GE_Damage）
UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
TSubclassOf<class UGameplayEffect> ContactDamageEffect;

// 单次接触伤害值
UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
float ContactDamage = 5.f;

// 接触伤害间隔（秒）
UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
float ContactDamageInterval = 0.5f;

// 接触伤害节拍累加器
float ContactDamageAccumulator = 0.f;

// 对目标施加一次接触伤害
void ApplyContactDamageTo(AActor* Target);
```

`EnemyBase.cpp` 顶部补 include：
```cpp
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "../Combat/ArenaCombatUtils.h"
#include "../GAS/GE_Damage.h"
#include "../GAS/ValorisGameplayTags.h"
```
构造函数末尾加：
```cpp
ContactDamageEffect = UGE_Damage::StaticClass();
```
`Tick` 在现有追玩家逻辑之后追加接触判定（沿用 120 贴身距离）：
```cpp
    // 贴身则按间隔对玩家施加接触伤害；离开接触重置节拍
    if (Distance <= 120.f)
    {
        if (ArenaCombat::AdvanceContactDamageTimer(ContactDamageAccumulator, DeltaTime, ContactDamageInterval))
        {
            ApplyContactDamageTo(Player);
        }
    }
    else
    {
        ContactDamageAccumulator = 0.f;
    }
```
新增函数（复用 melee 伤害管线）：
```cpp
void AEnemyBase::ApplyContactDamageTo(AActor* Target)
{
    if (!Target || !ContactDamageEffect)
    {
        return;
    }

    UAbilitySystemComponent* SelfASC = GetAbilitySystemComponent();
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    if (!SelfASC || !TargetASC)
    {
        return;
    }

    FGameplayEffectContextHandle Ctx = SelfASC->MakeEffectContext();
    Ctx.AddSourceObject(this);
    FGameplayEffectSpecHandle Spec = SelfASC->MakeOutgoingSpec(ContactDamageEffect, 1.f, Ctx);
    if (Spec.IsValid())
    {
        Spec.Data->SetSetByCallerMagnitude(FValorisGameplayTags::Data_Damage, ContactDamage);
        TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }
}
```

- [ ] **Step 9: 编译** → `Result: Succeeded`

- [ ] **Step 10: MCP / PIE 验证接触伤害**

开 PIE，让玩家站在敌人堆里不动，MCP 读玩家 ASC 血量随时间下降：
- 用 `SceneTools` 找到玩家 Pawn（`BP_Aric` 实例）的 actor refPath。
- 间隔多次调用 `GASToolsets.AbilitySystemInspectorToolset.GetAttributeValues {"actor":{"refPath":"<玩家Pawn>"}}`，读 `attributeName=="Health"` 的 `currentValue`。
- 预期：被贴身期间 `Health` 每 ~0.5s 下降 `ContactDamage - Defense`；玩家走开后停止下降。

- [ ] **Step 11: 提交**
```bash
git add Source/Valoris/Combat/ArenaCombatUtils.h Source/Valoris/Combat/ArenaCombatUtils.cpp Source/Valoris/Tests/ArenaCombatUtilsTest.cpp Source/Valoris/Enemy/EnemyBase.h Source/Valoris/Enemy/EnemyBase.cpp
git commit -m "feat(arena): 敌人贴身接触按间隔对玩家施加 GE_Damage（含节拍纯函数 + 测试）"
```

---

## 任务 4：玩家死亡 → 失败结算

**Files:**
- Modify: `Source/Valoris/Core/ValorisGameMode.cpp`（实现 `NotifyPlayerDied()`）
- Modify: `Source/Valoris/GAS/ValorisAttributeSet.cpp`（玩家血空→通知 GameMode）

**Interfaces:**
- Consumes: `AValorisGameMode::NotifyPlayerDied()`（声明已在任务 2 加）、`AValorisGameMode::OnGameOver`。

> GAS/世界集成，按验证哲学走「编译 + MCP/PIE」。

- [ ] **Step 1: 实现 `NotifyPlayerDied()`（`ValorisGameMode.cpp`）**

```cpp
void AValorisGameMode::NotifyPlayerDied()
{
    if (bGameOver)
    {
        return;
    }
    bGameOver = true;

    // 停止波次生成
    GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(WaveDelayTimerHandle);

    OnGameOver.Broadcast(false); // 失败
    UE_LOG(LogTemp, Warning, TEXT("Game Over - Defeat (player died)!"));
}
```

- [ ] **Step 2: 玩家血空镜像点（`ValorisAttributeSet.cpp`）**

顶部补 include：
```cpp
#include "../Character/AricHero.h"
#include "../Core/ValorisGameMode.h"
#include "Kismet/GameplayStatics.h"
```
`PostGameplayEffectExecute` 里现有的「血量为0触发死亡」块，把敌人分支扩成敌人/玩家二分支：
```cpp
                // 如果是敌人，标记为被击杀并销毁；如果是玩家，通知 GameMode 失败
                if (AEnemyBase* Enemy = Cast<AEnemyBase>(OwnerActor))
                {
                    Enemy->MarkAsKilled();
                    Enemy->Destroy();
                }
                else if (Cast<AAricHero>(OwnerActor))
                {
                    if (AValorisGameMode* GM = Cast<AValorisGameMode>(UGameplayStatics::GetGameMode(OwnerActor)))
                    {
                        GM->NotifyPlayerDied();
                    }
                }
```

- [ ] **Step 3: 编译** → `Result: Succeeded`

- [ ] **Step 4: MCP / PIE 验证失败流程**

开 PIE，让玩家被敌人围到血空（或站着不动等接触伤害磨死）：
- 预期 Output Log 出现 `Game Over - Defeat (player died)!`；
- 波次停止生成（不再有新敌人刷出）；
- `LogsToolset` 读 log 确认。

- [ ] **Step 5: 提交**
```bash
git add Source/Valoris/Core/ValorisGameMode.cpp Source/Valoris/GAS/ValorisAttributeSet.cpp
git commit -m "feat(arena): 玩家血量归零触发失败结算（AttributeSet 通知 GameMode 停波）"
```

---

## 任务 5：HUD —— 玩家血条 + 剩余敌人 + 第 N 波横幅（移除金币）

**Files:**
- Modify: `Source/Valoris/UI/ValorisHUD.h` / `.cpp`
- 编辑器：HUD WBP 绑定新控件

**Interfaces:**
- Consumes: 玩家 ASC 的 `UValorisAttributeSet::GetHealthAttribute()`/`GetMaxHealthAttribute()`；`AValorisGameMode::OnEnemyCountChanged`、`OnWaveStarted`、`GetAliveEnemyCount`、`GetTotalWaves`。

> UMG 集成，按验证哲学走「编译 + PIE 亲眼看」。

- [ ] **Step 1: 改 `ValorisHUD.h`**

删除金币相关：`UpdateGoldDisplay()`、`OnGoldChanged()`、`GoldText`。
新增（控件用 `BindWidgetOptional`，避免 WBP 未加控件时崩）：
```cpp
#include "Components/ProgressBar.h"   // 顶部
...
// 玩家血条
UPROPERTY(meta = (BindWidgetOptional))
TObjectPtr<class UProgressBar> PlayerHealthBar;

// 本波剩余敌人数文本
UPROPERTY(meta = (BindWidgetOptional))
TObjectPtr<UTextBlock> RemainingEnemiesText;

// "第 N 波" 横幅文本
UPROPERTY(meta = (BindWidgetOptional))
TObjectPtr<UTextBlock> WaveBannerText;
```
方法声明：
```cpp
void OnPlayerHealthChanged(const struct FOnAttributeChangeData& Data);
UFUNCTION() void OnEnemyCountChanged(int32 NewCount);
void RefreshPlayerHealth();
```

- [ ] **Step 2: 改 `ValorisHUD.cpp`**

顶部补 include：
```cpp
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "../GAS/ValorisAttributeSet.h"
#include "Components/ProgressBar.h"
```
`InitializeHUD()`：删除 ResourceManager/金币绑定与 `UpdateGoldDisplay` 初始化；新增绑定：
```cpp
    // 剩余敌人数
    GameMode->OnEnemyCountChanged.AddDynamic(this, &UValorisHUD::OnEnemyCountChanged);
    OnEnemyCountChanged(GameMode->GetAliveEnemyCount());

    // 玩家血条：绑玩家 ASC 的 Health 变化
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn))
        {
            ASC->GetGameplayAttributeValueChangeDelegate(UValorisAttributeSet::GetHealthAttribute())
                .AddUObject(this, &UValorisHUD::OnPlayerHealthChanged);
            RefreshPlayerHealth();
        }
    }

    // 隐藏波次横幅初始
    if (WaveBannerText)
    {
        WaveBannerText->SetVisibility(ESlateVisibility::Hidden);
    }
```
删除 `UpdateGoldDisplay`、`OnGoldChanged` 定义。新增：
```cpp
void UValorisHUD::OnEnemyCountChanged(int32 NewCount)
{
    if (RemainingEnemiesText)
    {
        RemainingEnemiesText->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewCount)));
    }
}

void UValorisHUD::OnPlayerHealthChanged(const FOnAttributeChangeData& Data)
{
    RefreshPlayerHealth();
}

void UValorisHUD::RefreshPlayerHealth()
{
    if (!PlayerHealthBar)
    {
        return;
    }
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    UAbilitySystemComponent* ASC = PlayerPawn ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn) : nullptr;
    if (!ASC)
    {
        return;
    }
    const float H = ASC->GetNumericAttribute(UValorisAttributeSet::GetHealthAttribute());
    const float MaxH = ASC->GetNumericAttribute(UValorisAttributeSet::GetMaxHealthAttribute());
    PlayerHealthBar->SetPercent(MaxH > 0.f ? H / MaxH : 0.f);
}
```
`OnWaveStarted()` 末尾加横幅显示（沿用现有波次文本刷新逻辑）：
```cpp
    if (WaveBannerText)
    {
        WaveBannerText->SetText(FText::FromString(FString::Printf(TEXT("第 %d 波"), WaveIndex + 1)));
        WaveBannerText->SetVisibility(ESlateVisibility::HitTestInvisible);
        // 2 秒后隐藏
        FTimerHandle BannerTimer;
        GetWorld()->GetTimerManager().SetTimer(BannerTimer, [this]()
        {
            if (WaveBannerText) { WaveBannerText->SetVisibility(ESlateVisibility::Hidden); }
        }, 2.0f, false);
    }
```

- [ ] **Step 3: 编译** → `Result: Succeeded`

- [ ] **Step 4: 编辑器 — HUD WBP 绑控件**

在 HUD 的 WBP 里：删金币文本；加 `PlayerHealthBar`(ProgressBar)、`RemainingEnemiesText`(TextBlock)、`WaveBannerText`(TextBlock，居中大字)，命名与 C++ `BindWidgetOptional` 完全一致。确认该 WBP 由 PlayerController/HUD 流程创建并 AddToViewport（M1 已有 HUD 显示则沿用）。

- [ ] **Step 5: PIE 验证**

开 PIE：血条随接触掉血而降；右上/指定位显示本波剩余敌人数随击杀递减；每波开始时屏幕中央闪「第 N 波」横幅 2 秒。

- [ ] **Step 6: 提交**
```bash
git add Source/Valoris/UI/ValorisHUD.h Source/Valoris/UI/ValorisHUD.cpp
git commit -m "feat(arena): HUD 玩家血条 + 本波剩余敌人 + 第N波横幅，移除金币显示"
```

---

## 任务 6：结算屏 + 重开

**Files:**
- Modify: `Source/Valoris/UI/ValorisHUD.h` / `.cpp`（结算面板 + 重开按钮）
- 编辑器：HUD WBP 加结算面板与按钮

**Interfaces:**
- Consumes: `AValorisGameMode::OnGameOver(bool)`、`AValorisGameMode::GetCurrentWaveIndex()`。

> UMG 集成，按验证哲学走「编译 + PIE」。

- [ ] **Step 1: 改 `ValorisHUD.h`**

加成员与方法：
```cpp
class UButton;
class UPanelWidget;
...
// 结算面板（胜负 + 撑过波数 + 重开）
UPROPERTY(meta = (BindWidgetOptional))
TObjectPtr<UPanelWidget> GameOverPanel;

UPROPERTY(meta = (BindWidgetOptional))
TObjectPtr<UTextBlock> GameOverResultText;

UPROPERTY(meta = (BindWidgetOptional))
TObjectPtr<UTextBlock> GameOverWavesText;

UPROPERTY(meta = (BindWidgetOptional))
TObjectPtr<UButton> RestartButton;

UFUNCTION()
void OnRestartClicked();
```

- [ ] **Step 2: 改 `ValorisHUD.cpp`**

顶部补 include：
```cpp
#include "Components/Button.h"
#include "Components/PanelWidget.h"
```
`InitializeHUD()` 末尾：隐藏结算面板 + 绑按钮：
```cpp
    if (GameOverPanel)
    {
        GameOverPanel->SetVisibility(ESlateVisibility::Hidden);
    }
    if (RestartButton)
    {
        RestartButton->OnClicked.AddDynamic(this, &UValorisHUD::OnRestartClicked);
    }
```
把 `OnGameOver()` 改为弹结算面板（取代/扩展现有 `ShowGameResult`）：
```cpp
void UValorisHUD::OnGameOver(bool bVictory)
{
    if (GameOverResultText)
    {
        GameOverResultText->SetText(FText::FromString(bVictory ? TEXT("胜利！") : TEXT("失败")));
    }
    if (GameOverWavesText)
    {
        AValorisGameMode* GameMode = Cast<AValorisGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
        const int32 Waves = GameMode ? (GameMode->GetCurrentWaveIndex() + 1) : 0;
        GameOverWavesText->SetText(FText::FromString(FString::Printf(TEXT("撑过 %d 波"), Waves)));
    }
    if (GameOverPanel)
    {
        GameOverPanel->SetVisibility(ESlateVisibility::Visible);
    }

    // 暂停 + 解锁鼠标，便于点重开
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->SetPause(true);
        PC->bShowMouseCursor = true;
        PC->SetInputMode(FInputModeUIOnly());
    }
}

void UValorisHUD::OnRestartClicked()
{
    // 解暂停后整关重载（永久死亡、不存档）
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        PC->SetPause(false);
    }
    const FName CurrentLevel(*UGameplayStatics::GetCurrentLevelName(GetWorld(), true));
    UGameplayStatics::OpenLevel(GetWorld(), CurrentLevel);
}
```
（`FInputModeUIOnly` 需 `#include "Blueprint/WidgetBlueprintLibrary.h"`？否——`FInputModeUIOnly` 在 `GameFramework/PlayerController.h` 已可用。`SetInputMode` 直接可用。）
删除旧的 `ShowGameResult()` 与 `OnAllWavesCompleted()` 里改文本的残留（若与新面板冲突，以面板为准；`OnAllWavesCompleted` 可保留只改 WaveText 文本，不冲突）。

- [ ] **Step 3: 编译** → `Result: Succeeded`

- [ ] **Step 4: 编辑器 — HUD WBP 加结算面板**

在 HUD WBP 里加一个默认隐藏的居中面板 `GameOverPanel`，内含 `GameOverResultText`、`GameOverWavesText`、`RestartButton`（按钮里放「重开」文本），命名与 C++ 一致。

- [ ] **Step 5: PIE 验证完整闭环（M2 验收）**

- 砍光 3 波 → 弹「胜利！撑过 3 波」面板 → 点「重开」→ 关卡重载、重新开局。
- 站着被磨死 → 弹「失败 撑过 X 波」面板 → 点「重开」→ 重新开局。
- 即 **M2 完成定义达成**。

- [ ] **Step 6: 提交**
```bash
git add Source/Valoris/UI/ValorisHUD.h Source/Valoris/UI/ValorisHUD.cpp
git commit -m "feat(arena): 极简结算屏（胜负+撑过波数）+ 重开按钮重载关卡，M2 闭环达成"
```

---

## 完成定义（M2 验收）

顶下视角下，3 波递增的敌人从玩家四周环形涌来；玩家用 WASD+鼠标+左键清怪，被围会掉血；血空弹「失败」结算屏、砍光 3 波弹「胜利」结算屏，点「重开」整关重载新开一局。HUD 实时显示玩家血条、当前/总波数、本波剩余敌人、每波开始的「第 N 波」横幅。

## 风险/兜底

- **延迟开波取不到玩家**：已用 0.5s 延迟 timer；若仍取不到可加重试。
- **删基地血量牵连编译**：任务 2 已在同任务内同步清掉 HUD 的基地血量引用；编译前用 Grep 复查 `BaseHealth`/`DamageBase`/`OnBaseHealthChanged` 残留。
- **接触伤害一帧多跳**：每敌人独立 `ContactDamageAccumulator` 按间隔施加，已测节拍。
- **玩家死亡分支误判**：`Cast<AAricHero>` 精确匹配玩家；敌人分支在前，互斥。
- **重开后输入模式残留**：重开走 `OpenLevel` 整关重载，天然清状态；`OnRestartClicked` 先 `SetPause(false)`。
- **WBP 控件名不匹配**：全部用 `BindWidgetOptional`，名字对不上不崩、只是该项不显示，PIE 里逐项核对命名。

## 执行顺序与测试粒度

任务 1→6 顺序执行。任务 1、3 的纯函数走完整 TDD（RED→GREEN）。每个集成任务编译通过 + 该任务 PIE/MCP 验证项达标即可进入下一个；任务 6 PIE 跑通完整闭环作为 M2 final gate。
