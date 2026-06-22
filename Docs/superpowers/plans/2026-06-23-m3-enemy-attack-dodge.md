# M3 实现计划：敌人主动攻击 + 闪避无敌帧 + 3 种敌人

> **For agentic workers:** REQUIRED SUB-SKILL: 用 superpowers:subagent-driven-development（推荐）或 superpowers:executing-plans 逐任务实现本计划。步骤用 `- [ ]` 复选框跟踪。

**Goal:** 把 M2 的「贴身持续掉血」升级为「敌人起手前摇、挥击一次性结算的真攻击」，玩家空格朝移动方向翻滚带无敌帧可完整躲掉，并上 3 种数值不同的敌人（普通/快速/坦克）。

**Architecture:** 敌人攻击逻辑用一个相位状态机（Approaching/WindingUp/Recovering）扩展 `EnemyBase::Tick`，相位推进与命中判定抽成 `ArenaCombat` 命名空间的纯函数上 Automation 测试；前摇用代码驱动的 scale-pulse 表现（不依赖动画资产）。闪避把现有 `GA_Charge` 改造重命名为 `GA_Dodge`（方向取移动输入、翻滚期挂 `State.Invincible` 无敌标签、去掉撞击伤害）。伤害收口到 `ExecCalc_Damage`（修掉 `PostGameplayEffectExecute` 重复减防御的既有 bug，并在目标带无敌标签时早退）。三种敌人用同一套 `EnemyBase` 派生 3 个蓝图调参，数据驱动。

**Tech Stack:** UE 5.8、C++、GAS（GameplayEffect/AttributeSet/ASC/SetByCaller/GameplayTag）、Enhanced Input、UE Automation Test。

## Global Constraints

- 引擎：UE 5.8，二进制 Launcher 引擎在 `G:\Unreal Engines\UE_5.8`。
- **编译命令**（关闭编辑器后跑；新增 UPROPERTY/类/文件必须完整重编，Live Coding 编不进）：
  `& "G:\Unreal Engines\UE_5.8\Engine\Build\BatchFiles\Build.bat" ValorisEditor Win64 Development -project="F:\Valoris\Valoris.uproject" -waitmutex`
- **Automation 测试跑法**（二选一）：① MCP `AutomationTestToolset` DiscoverTests→RunTests({TestNames:[...]})；② headless：
  `& "G:\Unreal Engines\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "F:\Valoris\Valoris.uproject" -ExecCmds="Automation RunTests <TestNamePrefix>" -TestExit="Automation Test Queue Empty" -nullrhi -unattended -nopause -log`
- **UE 现实混合执行流**（沿用 M1/M2）：实现子代理只写 C++/测试代码并提交；**编译（关编辑器 Build.bat）、跑 Automation、MCP/PIE 验证、编辑器内资产配置（BP/DataAsset/输入资产）由主会话 + 用户做**。纯逻辑任务里 RED/GREEN 由主会话编译后跑 Automation 确认；集成任务靠 MCP/PIE 验证、不写单测（验证哲学：集成任务不写单测、别把没单测当缺陷误报）。
- **TDD 铁律**：有业务逻辑的纯函数必须先写失败测试、跑出红、再写最小实现转绿。
- 命名/目录沿用现有规范（GA_/GE_ 前缀，tag 走 `FValorisGameplayTags` 不硬编码字符串）。
- **删除而非注释**：被取代的死代码（M2 接触伤害、GA_Charge 撞击伤害、PostGameplayEffectExecute 重复减防御）直接删，删前 Grep 确认无残留引用。
- 提交信息用中文，conventional commit 前缀保留英文；不加 AI 署名。每任务独立提交。
- spec：`Docs/superpowers/specs/2026-06-23-m3-enemy-attack-dodge-design.md`。

---

## 文件结构（要动的文件 + 各自职责）

**C++ 源码：**
- `Source/Valoris/Combat/ArenaCombatUtils.h/.cpp`（改）— 加 `ComputeMitigatedDamage`、`EEnemyAttackPhase` 枚举、`FEnemyAttackStep` 结构、`StepEnemyAttack`、`ShouldEnemyStrikeConnect`；删不再被引用的 `AdvanceContactDamageTimer`。
- `Source/Valoris/Tests/ArenaCombatUtilsTest.cpp`（改）— 加新纯函数的 Automation 测试；删 `AdvanceContactDamageTimer` 测试。
- `Source/Valoris/GAS/ExecCalc_Damage.cpp`（改）— 用 `ComputeMitigatedDamage` 算减伤；目标带 `State.Invincible` 时早退 0 伤害。
- `Source/Valoris/GAS/ValorisAttributeSet.cpp`（改）— 删 `PostGameplayEffectExecute` 重复减防御。
- `Source/Valoris/GAS/ValorisGameplayTags.h/.cpp`（改）— 加 `State_Invincible`；`Ability_Movement_Aric_Charge` → `Ability_Movement_Aric_Dodge`。
- `Source/Valoris/Enemy/EnemyBase.h/.cpp`（改）— 删接触伤害整套；加攻击状态机字段 + 参数 + `ApplyMeleeDamageTo` + scale-pulse 前摇。
- `Source/Valoris/GAS/Abilities/Movement/GA_Charge.{h,cpp}` → 重命名为 `GA_Dodge.{h,cpp}`（改）— 类 `UGA_Charge`→`UGA_Dodge`，方向取移动输入、挂无敌标签、删撞击伤害。
- `Source/Valoris/Character/AricHero.h/.cpp`（改）— 加 `DodgeAction` + 空格绑定 + `OnDodgeInput` 激活闪避。
- `Config/DefaultEngine.ini`（改）— 加 `GA_Charge`→`GA_Dodge` 类重定向，保护既有资产引用。

**资产（编辑器/MCP 配置，主会话+用户做）：**
- `BP_EnemyBasic`（= 普通）参数调校；新建 `BP_EnemyFast`、`BP_EnemyTank`。
- `DA_ArenaWaves`：3 种敌人混编。
- `IA_Dodge`（Input Action）+ 加进 IMC（空格）+ 赋给 `BP_Aric.DodgeAction`。
- `BP_Aric.DefaultAbilities` 加入闪避技能 BP（`BP_GA_Dodge` 或重定向后的 GA_Charge BP）。
- 闪避冷却（可选）：`GA_Dodge` 的 Cooldown GE。

---

## Task 1：修复双重扣防御 + `ComputeMitigatedDamage` 纯函数

**Files:**
- Modify: `Source/Valoris/Combat/ArenaCombatUtils.h`、`Source/Valoris/Combat/ArenaCombatUtils.cpp`
- Modify: `Source/Valoris/Tests/ArenaCombatUtilsTest.cpp`
- Modify: `Source/Valoris/GAS/ExecCalc_Damage.cpp:48`
- Modify: `Source/Valoris/GAS/ValorisAttributeSet.cpp:55`
- Test: `Source/Valoris/Tests/ArenaCombatUtilsTest.cpp`

**Interfaces:**
- Produces: `float ArenaCombat::ComputeMitigatedDamage(float BaseDamage, float Defense)` —— 返回 `FMath::Max(1.f, BaseDamage - Defense)`，伤害减伤的唯一收口。

- [ ] **Step 1：写失败测试**（在 `ArenaCombatUtilsTest.cpp` 顶部 include 区已有 `#include "../Combat/ArenaCombatUtils.h"`，在 `#if WITH_DEV_AUTOMATION_TESTS` 块内追加）

```cpp
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
```

- [ ] **Step 2：跑测试确认失败**

Run（主会话，关编辑器后 Build.bat，再跑）：`Automation RunTests Valoris.ArenaCombat.ComputeMitigatedDamage`
Expected：编译失败 `ComputeMitigatedDamage` 未声明（RED）。

- [ ] **Step 3：写最小实现**（`ArenaCombatUtils.h` 的 `namespace ArenaCombat` 内加声明）

```cpp
    // 伤害减伤的唯一收口：基础伤害减去防御，最低为 1。
    VALORIS_API float ComputeMitigatedDamage(float BaseDamage, float Defense);
```

`ArenaCombatUtils.cpp` 内加定义：

```cpp
float ArenaCombat::ComputeMitigatedDamage(float BaseDamage, float Defense)
{
    return FMath::Max(1.f, BaseDamage - Defense);
}
```

- [ ] **Step 4：跑测试确认通过**

Run：`Automation RunTests Valoris.ArenaCombat.ComputeMitigatedDamage`
Expected：`ComputeMitigatedDamage` 状态 Success（GREEN）。

- [ ] **Step 5：ExecCalc 改用纯函数**（`ExecCalc_Damage.cpp`）

把第 48 行：
```cpp
	float FinalDamage = FMath::Max(1.f, BaseDamage - Defense);
```
改为（顶部 include 区加 `#include "../Combat/ArenaCombatUtils.h"`）：
```cpp
	float FinalDamage = ArenaCombat::ComputeMitigatedDamage(BaseDamage, Defense);
```

- [ ] **Step 6：删 `PostGameplayEffectExecute` 重复减防御**（`ValorisAttributeSet.cpp`）

把第 55 行：
```cpp
			const float ActualDamage = FMath::Max(Damage - GetDefense(), 0.f);
```
改为（伤害已在 ExecCalc 减过防御，这里直接用）：
```cpp
			// 伤害已在 ExecCalc_Damage 减过防御，这里不再二次减（修复双重扣防御 bug）
			const float ActualDamage = Damage;
```

- [ ] **Step 7：编译确认无报错**

Run：Build.bat（见 Global Constraints）。
Expected：编译成功，`Valoris.ArenaCombat.ComputeMitigatedDamage` 仍 Success。

- [ ] **Step 8：提交**

```bash
git add Source/Valoris/Combat/ArenaCombatUtils.h Source/Valoris/Combat/ArenaCombatUtils.cpp Source/Valoris/Tests/ArenaCombatUtilsTest.cpp Source/Valoris/GAS/ExecCalc_Damage.cpp Source/Valoris/GAS/ValorisAttributeSet.cpp
git commit -m "fix(arena): 伤害减伤收口到 ComputeMitigatedDamage 纯函数，修双重扣防御 bug"
```

---

## Task 2：敌人攻击纯逻辑（相位状态机 + 命中判定）

**Files:**
- Modify: `Source/Valoris/Combat/ArenaCombatUtils.h`、`Source/Valoris/Combat/ArenaCombatUtils.cpp`
- Test: `Source/Valoris/Tests/ArenaCombatUtilsTest.cpp`

**Interfaces:**
- Consumes: 无（纯逻辑）。
- Produces:
  - `enum class ArenaCombat::EEnemyAttackPhase : uint8 { Approaching, WindingUp, Recovering }`
  - `struct ArenaCombat::FEnemyAttackStep { EEnemyAttackPhase Phase; bool bStrike; }`
  - `FEnemyAttackStep ArenaCombat::StepEnemyAttack(EEnemyAttackPhase Current, float& Accumulator, float DeltaTime, float WindupDuration, float RecoveryDuration, bool bInAttackRange)` —— 推进一帧攻击相位；返回下一相位 + 本帧是否触发挥击。状态转移：Approaching+进入距离→WindingUp（清零累加器）；WindingUp+离开距离→打断回 Approaching；WindingUp 累加达 WindupDuration→Recovering 且 bStrike=true；Recovering 累加达 RecoveryDuration→Approaching。
  - `bool ArenaCombat::ShouldEnemyStrikeConnect(float DistanceToTarget, float AttackReach, bool bTargetInvincible)` —— `!bTargetInvincible && DistanceToTarget <= AttackReach`。

- [ ] **Step 1：写失败测试**（`ArenaCombatUtilsTest.cpp` 追加）

```cpp
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
```

- [ ] **Step 2：跑测试确认失败**

Run：`Automation RunTests Valoris.ArenaCombat.StepEnemyAttack`
Expected：编译失败（`StepEnemyAttack`/`EEnemyAttackPhase` 未声明，RED）。

- [ ] **Step 3：写最小实现**（`ArenaCombatUtils.h` 的 `namespace ArenaCombat` 内加）

```cpp
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
```

`ArenaCombatUtils.cpp` 内加定义：

```cpp
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
```

- [ ] **Step 4：跑测试确认通过**

Run：`Automation RunTests Valoris.ArenaCombat.StepEnemyAttack` 和 `Valoris.ArenaCombat.ShouldEnemyStrikeConnect`
Expected：两个测试 Success（GREEN）。

- [ ] **Step 5：提交**

```bash
git add Source/Valoris/Combat/ArenaCombatUtils.h Source/Valoris/Combat/ArenaCombatUtils.cpp Source/Valoris/Tests/ArenaCombatUtilsTest.cpp
git commit -m "feat(arena): 敌人攻击相位状态机 + 命中判定纯函数（TDD）"
```

---

## Task 3：`State.Invincible` 标签 + ExecCalc 无敌早退

**Files:**
- Modify: `Source/Valoris/GAS/ValorisGameplayTags.h`、`Source/Valoris/GAS/ValorisGameplayTags.cpp`
- Modify: `Source/Valoris/GAS/ExecCalc_Damage.cpp`

**Interfaces:**
- Produces: `static FGameplayTag FValorisGameplayTags::State_Invincible`（tag `State.Invincible`）。被 Task 4（敌人挥击查询）、Task 5（闪避挂载）消费。

- [ ] **Step 1：声明 tag**（`ValorisGameplayTags.h`，在状态标签区 `State_Buffed` 后加）

```cpp
	static FGameplayTag State_Invincible;		// State.Invincible - 无敌（闪避无敌帧）
```

- [ ] **Step 2：定义并注册 tag**（`ValorisGameplayTags.cpp`）

在静态成员定义区（`State_Buffed;` 后）加：
```cpp
FGameplayTag FValorisGameplayTags::State_Invincible;
```
在 `InitializeNativeTags()` 状态标签区（`State_Buffed = ...` 之后）加：
```cpp
	State_Invincible = Manager.AddNativeGameplayTag(
		FName("State.Invincible"),
		FString("Character is invincible (dodge i-frames)")
	);
```

- [ ] **Step 3：ExecCalc 无敌早退**（`ExecCalc_Damage.cpp`，`Execute_Implementation` 内，取到 `EvaluateParams` 之后、计算伤害之前）

在第 38 行（`EvaluateParams.TargetTags = ...`）之后加：
```cpp
	// 目标处于无敌（闪避无敌帧）则不结算任何伤害
	if (EvaluateParams.TargetTags && EvaluateParams.TargetTags->HasTag(FValorisGameplayTags::State_Invincible))
	{
		return;
	}
```
（`ValorisGameplayTags.h` 已在该文件 include。）

- [ ] **Step 4：编译确认无报错**

Run：Build.bat。
Expected：编译成功（tag 已注册、ExecCalc 引用通过）。

- [ ] **Step 5：提交**

```bash
git add Source/Valoris/GAS/ValorisGameplayTags.h Source/Valoris/GAS/ValorisGameplayTags.cpp Source/Valoris/GAS/ExecCalc_Damage.cpp
git commit -m "feat(arena): 加 State.Invincible 标签 + ExecCalc 对无敌目标早退 0 伤害"
```

---

## Task 4：`EnemyBase` 攻击状态机 + scale-pulse 前摇 + 删接触伤害

**Files:**
- Modify: `Source/Valoris/Enemy/EnemyBase.h`、`Source/Valoris/Enemy/EnemyBase.cpp`

**Interfaces:**
- Consumes: `ArenaCombat::EEnemyAttackPhase`、`ArenaCombat::StepEnemyAttack`、`ArenaCombat::ShouldEnemyStrikeConnect`（Task 2）；`FValorisGameplayTags::State_Invincible`（Task 3）。
- Produces: `EnemyBase` 的 `EditDefaultsOnly` 参数 `AttackRange`/`AttackReach`/`AttackDamage`/`WindupDuration`/`RecoveryDuration`/`MeleeDamageEffect`（供 Task 6 三个 BP 调校）。

- [ ] **Step 1：改 `EnemyBase.h` —— 删接触伤害字段，加攻击状态机字段**

把「接触伤害」整段（`ContactDamageEffect`/`ContactDamage`/`ContactDamageInterval`/`ContactDamageAccumulator`/`ApplyContactDamageTo` 声明，第 99-117 行）替换为：

```cpp
	// ========== 近战攻击 ==========

	// 挥击施加的 GameplayEffect（默认 GE_Damage）
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	TSubclassOf<class UGameplayEffect> MeleeDamageEffect;

	// 进入此距离则停下起手前摇
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float AttackRange = 150.f;

	// 挥击命中触及距离（前摇结束时玩家在此距离内才吃伤害）
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float AttackReach = 180.f;

	// 单次挥击伤害
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float AttackDamage = 12.f;

	// 前摇时长（秒）
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float WindupDuration = 0.4f;

	// 挥击后冷却时长（秒）
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float RecoveryDuration = 0.5f;

	// 当前攻击相位（运行时状态，非编辑器编辑）
	ArenaCombat::EEnemyAttackPhase CurrentAttackPhase = ArenaCombat::EEnemyAttackPhase::Approaching;

	// 当前相位内累计时间
	float AttackAccumulator = 0.f;

	// Mesh 初始相对缩放（前摇放大用，BeginPlay 缓存）
	FVector InitialMeshScale = FVector::OneVector;

	// 对目标施加一次挥击伤害
	void ApplyMeleeDamageTo(AActor* Target);

	// 目标是否处于无敌
	bool IsTargetInvincible(AActor* Target) const;

	// 按当前相位刷新前摇视觉（scale pulse）
	void UpdateWindupVisual();
```

在 `EnemyBase.h` 顶部 include 区（`#include "../Character/ValorisCharacterBase.h"` 后）加：
```cpp
#include "../Combat/ArenaCombatUtils.h"
```

- [ ] **Step 2：改 `EnemyBase.cpp` 构造函数**

把第 33 行：
```cpp
	ContactDamageEffect = UGE_Damage::StaticClass();
```
改为：
```cpp
	MeleeDamageEffect = UGE_Damage::StaticClass();
```

- [ ] **Step 3：`BeginPlay` 缓存 Mesh 初始缩放**

`BeginPlay()` 内 `InitializeHealthBar();` 之后加：
```cpp
	if (GetMesh())
	{
		InitialMeshScale = GetMesh()->GetRelativeScale3D();
	}
```

- [ ] **Step 4：重写 `Tick`（攻击状态机）**

把整个 `Tick`（第 64-100 行）替换为：

```cpp
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}

	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const float Distance = ToPlayer.Size();
	const bool bInAttackRange = Distance <= AttackRange;

	// 仅 Approaching 且未进入攻击距离时追击移动
	if (CurrentAttackPhase == ArenaCombat::EEnemyAttackPhase::Approaching && !bInAttackRange)
	{
		const FVector Dir = ToPlayer.GetSafeNormal();
		AddMovementInput(Dir, 1.f);
		SetActorRotation(FRotator(0.f, Dir.Rotation().Yaw, 0.f));
	}

	// 推进攻击状态机
	const ArenaCombat::FEnemyAttackStep Step = ArenaCombat::StepEnemyAttack(
		CurrentAttackPhase, AttackAccumulator, DeltaTime, WindupDuration, RecoveryDuration, bInAttackRange);
	CurrentAttackPhase = Step.Phase;

	// 前摇结束的挥击
	if (Step.bStrike)
	{
		if (ArenaCombat::ShouldEnemyStrikeConnect(Distance, AttackReach, IsTargetInvincible(Player)))
		{
			ApplyMeleeDamageTo(Player);
		}
	}

	UpdateWindupVisual();
}
```

- [ ] **Step 5：把 `ApplyContactDamageTo` 改名实现为 `ApplyMeleeDamageTo` + 加 `IsTargetInvincible`/`UpdateWindupVisual`**

把 `ApplyContactDamageTo`（第 102-124 行）整体替换为：

```cpp
void AEnemyBase::ApplyMeleeDamageTo(AActor* Target)
{
	if (!Target || !MeleeDamageEffect)
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
	FGameplayEffectSpecHandle Spec = SelfASC->MakeOutgoingSpec(MeleeDamageEffect, 1.f, Ctx);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(FValorisGameplayTags::Data_Damage, AttackDamage);
		TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

bool AEnemyBase::IsTargetInvincible(AActor* Target) const
{
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
	{
		return TargetASC->HasMatchingGameplayTag(FValorisGameplayTags::State_Invincible);
	}
	return false;
}

void AEnemyBase::UpdateWindupVisual()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	if (CurrentAttackPhase == ArenaCombat::EEnemyAttackPhase::WindingUp)
	{
		const float Alpha = WindupDuration > 0.f ? FMath::Clamp(AttackAccumulator / WindupDuration, 0.f, 1.f) : 1.f;
		MeshComp->SetRelativeScale3D(InitialMeshScale * FMath::Lerp(1.f, 1.15f, Alpha));
	}
	else
	{
		MeshComp->SetRelativeScale3D(InitialMeshScale);
	}
}
```

- [ ] **Step 6：清理无引用的 include 与接触伤害纯函数**

- `EnemyBase.cpp` 顶部移除不再需要的 `#include "../Combat/ArenaCombatUtils.h"` 重复（若 Step 1 已在 .h 引入，.cpp 可保留或去重，确保编译通过）。
- Grep 确认 `AdvanceContactDamageTimer` 已无任何引用：

Run：`rg AdvanceContactDamageTimer Source`
Expected：仅 `ArenaCombatUtils.h/.cpp` 与 `ArenaCombatUtilsTest.cpp` 命中。

- 删除 `ArenaCombatUtils.h` 中 `AdvanceContactDamageTimer` 声明、`ArenaCombatUtils.cpp` 中其定义、`ArenaCombatUtilsTest.cpp` 中 `FArenaContactDamageTimerTest` 整个测试块。

- [ ] **Step 7：编译确认无报错**

Run：Build.bat。
Expected：编译成功，无 `ContactDamage`/`AdvanceContactDamageTimer` 残留引用报错。

- [ ] **Step 8：MCP/PIE 集成验证（主会话）**

- 关编辑器编译完 → 开编辑器 → MCP `StartPIE`。
- `SceneTools.find_actors(actor_type=EnemyBase)` 找到敌人；观察其靠近玩家后停下、Mesh 放大（前摇）→ 复位（挥击）。
- `AbilitySystemInspectorToolset.GetAttributeValues` 读玩家 `Health`：站着不躲 → 每轮挥击掉一块（约 `AttackDamage - Defense`）；远离时不掉血。
- `LogsToolset.GetLogEntries` 无报错。

- [ ] **Step 9：提交**

```bash
git add Source/Valoris/Enemy/EnemyBase.h Source/Valoris/Enemy/EnemyBase.cpp Source/Valoris/Combat/ArenaCombatUtils.h Source/Valoris/Combat/ArenaCombatUtils.cpp Source/Valoris/Tests/ArenaCombatUtilsTest.cpp
git commit -m "feat(arena): 敌人前摇攻击状态机接入 EnemyBase + scale-pulse 前摇，删 M2 接触伤害"
```

---

## Task 5：`GA_Charge` → `GA_Dodge` 改造 + AricHero 空格闪避

**Files:**
- Rename: `Source/Valoris/GAS/Abilities/Movement/GA_Charge.h` → `GA_Dodge.h`，`GA_Charge.cpp` → `GA_Dodge.cpp`
- Modify（重命名后）: `GA_Dodge.h`、`GA_Dodge.cpp`
- Modify: `Source/Valoris/GAS/ValorisGameplayTags.h`、`Source/Valoris/GAS/ValorisGameplayTags.cpp`
- Modify: `Source/Valoris/Character/AricHero.h`、`Source/Valoris/Character/AricHero.cpp`
- Modify: `Config/DefaultEngine.ini`

**Interfaces:**
- Consumes: `FValorisGameplayTags::State_Invincible`（Task 3）。
- Produces: `UGA_Dodge`（GameplayAbility，tag `Ability.Movement.Aric.Dodge`），翻滚期间给 Avatar ASC 挂 `State.Invincible`，方向取角色移动输入。`AAricHero::DodgeAction`（供 BP 指向 IA_Dodge）。

- [ ] **Step 1：tag 重命名 Charge→Dodge**

`ValorisGameplayTags.h` 第 43 行：
```cpp
	static FGameplayTag Ability_Movement_Aric_Charge;				// Ability.Movement.Aric.Charge - Aric 冲锋 (E)
```
改为：
```cpp
	static FGameplayTag Ability_Movement_Aric_Dodge;				// Ability.Movement.Aric.Dodge - Aric 闪避翻滚 (空格)
```
`ValorisGameplayTags.cpp` 静态定义第 22 行 `Ability_Movement_Aric_Charge;` 改为 `Ability_Movement_Aric_Dodge;`；`InitializeNativeTags()` 内（第 96-99 行）改为：
```cpp
	Ability_Movement_Aric_Dodge = Manager.AddNativeGameplayTag(
		FName("Ability.Movement.Aric.Dodge"),
		FString("Aric dodge roll ability (Space) - movement with i-frames")
	);
```

- [ ] **Step 2：重命名文件 + 类**

```bash
git mv Source/Valoris/GAS/Abilities/Movement/GA_Charge.h Source/Valoris/GAS/Abilities/Movement/GA_Dodge.h
git mv Source/Valoris/GAS/Abilities/Movement/GA_Charge.cpp Source/Valoris/GAS/Abilities/Movement/GA_Dodge.cpp
```

- [ ] **Step 3：重写 `GA_Dodge.h`**（整体替换为）

```cpp
// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../../GA_MontageAbilityBase.h"
#include "GA_Dodge.generated.h"

/**
 * 闪避翻滚技能
 * 朝玩家移动输入方向快速位移，期间挂 State.Invincible 无敌帧，无伤害。
 */
UCLASS()
class VALORIS_API UGA_Dodge : public UGA_MontageAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Dodge();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	virtual void OnEventReceived(FGameplayEventData Payload) override;

	/** 翻滚距离 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float DodgeDistance = 500.0f;

	/** 翻滚速度 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float DodgeSpeed = 2000.0f;

	/** 翻滚持续时间（<=0 则按距离/速度自动计算） */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float DodgeDuration = 0.0f;

private:
	/** 翻滚方向 */
	FVector DodgeDirection;

	/** 已位移距离 */
	float DodgedDistance;

	/** 执行翻滚位移 */
	void PerformDodge();

	/** 翻滚定时器句柄 */
	FTimerHandle DodgeTimerHandle;

	/** 翻滚结束 */
	void OnDodgeEnd();
};
```

- [ ] **Step 4：重写 `GA_Dodge.cpp`**（整体替换为）

```cpp
// Copyright Valoris. All Rights Reserved.

#include "GA_Dodge.h"
#include "AbilitySystemComponent.h"
#include "../../ValorisGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UGA_Dodge::UGA_Dodge()
{
}

void UGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 方向取角色当前移动输入（WASD），零输入回退角色朝向
	FVector InputDir = FVector::ZeroVector;
	if (ACharacter* Char = Cast<ACharacter>(AvatarActor))
	{
		if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
		{
			InputDir = Move->GetLastInputVector();
		}
	}
	InputDir.Z = 0.f;
	if (InputDir.IsNearlyZero())
	{
		InputDir = AvatarActor->GetActorForwardVector();
	}
	DodgeDirection = InputDir.GetSafeNormal();
	DodgedDistance = 0.0f;

	// 挂无敌帧
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(FValorisGameplayTags::State_Invincible);
	}

	float Duration = DodgeDuration;
	if (Duration <= 0.0f)
	{
		Duration = DodgeDistance / DodgeSpeed;
	}

	// 播放动画（如有）
	if (AbilityMontage)
	{
		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DodgeTimerHandle, this, &UGA_Dodge::PerformDodge, 0.016f, true);

		FTimerHandle EndTimerHandle;
		World->GetTimerManager().SetTimer(EndTimerHandle, this, &UGA_Dodge::OnDodgeEnd, Duration, false);
	}
}

void UGA_Dodge::OnEventReceived(FGameplayEventData Payload)
{
	// 闪避不依赖 AnimNotify 事件
}

void UGA_Dodge::PerformDodge()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		OnDodgeEnd();
		return;
	}

	const float DeltaTime = 0.016f;
	float MoveDistance = DodgeSpeed * DeltaTime;
	if (DodgedDistance + MoveDistance > DodgeDistance)
	{
		MoveDistance = DodgeDistance - DodgedDistance;
	}

	const FVector NewLocation = AvatarActor->GetActorLocation() + DodgeDirection * MoveDistance;
	AvatarActor->SetActorLocation(NewLocation, true);
	DodgedDistance += MoveDistance;

	if (DodgedDistance >= DodgeDistance)
	{
		OnDodgeEnd();
	}
}

void UGA_Dodge::OnDodgeEnd()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DodgeTimerHandle);
	}

	// 移除无敌帧
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FValorisGameplayTags::State_Invincible);
	}

	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo, GetCurrentActivationInfo(), true, false);
}
```

- [ ] **Step 5：AricHero 加空格闪避输入**

`AricHero.h`：在 `OnAttackInput();`（第 29 行）后加：
```cpp
	// 空格闪避输入（激活闪避技能）
	void OnDodgeInput();
```
在 `AttackAction`（第 45 行）后加：
```cpp
	// 闪避 InputAction（编辑器里指向空格 IA_Dodge）
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputAction> DodgeAction;
```

`AricHero.cpp`：`SetupPlayerInputComponent` 内（`AttackAction` 绑定块后）加：
```cpp
			if (DodgeAction)
			{
				EIC->BindAction(DodgeAction, ETriggerEvent::Started, this, &AAricHero::OnDodgeInput);
			}
```
文件末尾加：
```cpp
void AAricHero::OnDodgeInput()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayTagContainer AbilityTags;
		AbilityTags.AddTag(FValorisGameplayTags::Ability_Movement_Aric_Dodge);
		ASC->TryActivateAbilitiesByTag(AbilityTags);
	}
}
```

- [ ] **Step 6：加类重定向保护既有资产引用**（`Config/DefaultEngine.ini`）

在文件末尾追加（若已有 `[CoreRedirects]` 段则并入）：
```ini
[CoreRedirects]
+ClassRedirects=(OldName="/Script/Valoris.GA_Charge",NewName="/Script/Valoris.GA_Dodge")
```

- [ ] **Step 7：Grep 确认无 GA_Charge / Charge tag 源码残留**

Run：`rg "GA_Charge|Ability_Movement_Aric_Charge|UGA_Charge" Source`
Expected：无命中（全部已改名）。

- [ ] **Step 8：编译确认无报错**

Run：Build.bat。
Expected：编译成功。

- [ ] **Step 9：编辑器/MCP 资产配置（主会话+用户）**

- 若存在 `BP_GA_Charge` 资产：重命名为 `BP_GA_Dodge`，确认 Reparent 到 `UGA_Dodge`（类重定向已保护引用）；否则新建 `BP_GA_Dodge`（父类 `GA_Dodge`），设其 `AbilityTags` 含 `Ability.Movement.Aric.Dodge`，`AbilityMontage` 指向 Aric 翻滚动画（有则填，无则留空，M5 补）。
- 新建 `IA_Dodge`（Input Action，Digital bool）；在 Aric 用的 `IMC` 里把 **空格** 映射到 `IA_Dodge`。
- `BP_Aric`：`DodgeAction` 指向 `IA_Dodge`；`DefaultAbilities` 数组加入 `BP_GA_Dodge`。
- （可选冷却）给 `BP_GA_Dodge` 的 Cooldown GE 配一个 ~0.8s 的 `GE_Cooldown`，防连滚。
- BP 改完 `compile_blueprint` + `save_assets`。

- [ ] **Step 10：MCP/PIE 集成验证（主会话）**

- `StartPIE` → 按空格：角色朝当前 WASD 方向（无输入时朝鼠标朝向）快速翻滚一段。
- 让敌人挥击的同时按空格穿过：`GetAttributeValues` 读玩家 `Health` 不变（无敌帧生效）；不躲则掉血。
- `GetLogEntries` 无报错。

- [ ] **Step 11：提交**

```bash
git add Source/Valoris/GAS/Abilities/Movement/GA_Dodge.h Source/Valoris/GAS/Abilities/Movement/GA_Dodge.cpp Source/Valoris/GAS/ValorisGameplayTags.h Source/Valoris/GAS/ValorisGameplayTags.cpp Source/Valoris/Character/AricHero.h Source/Valoris/Character/AricHero.cpp Config/DefaultEngine.ini
git commit -m "feat(arena): GA_Charge 改造为 GA_Dodge（移动方向翻滚+无敌帧+去伤害）+ AricHero 空格闪避"
```

---

## Task 6：3 种敌人 BP + `DA_ArenaWaves` 混编（资产/MCP）

> 纯资产配置任务，无 C++、无单测；靠 MCP/PIE 验证。BP CDO 改默认值后必须 `compile_blueprint` 才被 PIE 实例化认（项目 MCP 工作流）。

**Files:**
- 资产: `BP_EnemyBasic`（= 普通，调参）、`BP_EnemyFast`（新建）、`BP_EnemyTank`（新建）、`DA_ArenaWaves`（改）

- [ ] **Step 1：普通 EnemyNormal 调参**（`BP_EnemyBasic`，MCP `ObjectTools.set_properties` 改 CDO 默认值）

- `AttackRange=150`、`AttackReach=180`、`AttackDamage=12`、`WindupDuration=0.4`、`RecoveryDuration=0.5`。
- 血量/移速沿用现有普通值（移速 `MaxWalkSpeed` 约 450~500）。
- `compile_blueprint` + `save_assets`。

- [ ] **Step 2：新建快速 EnemyFast**

- 复制 `BP_EnemyBasic` 为 `BP_EnemyFast`（父类同 `EnemyBase` 或继承 `BP_EnemyBasic`）。
- 调参：血量低（约普通 0.6×）、`MaxWalkSpeed` 高（约 700）、`AttackDamage=8`、`WindupDuration=0.25`、`RecoveryDuration=0.4`。
- `compile_blueprint` + `save_assets`。

- [ ] **Step 3：新建坦克 EnemyTank**

- 复制为 `BP_EnemyTank`。
- 调参：血量高（约普通 2.5×）、`MaxWalkSpeed` 低（约 280）、`AttackDamage=22`、`WindupDuration=0.7`、`RecoveryDuration=0.7`、`AttackRange/Reach` 略大（180/210）。
- `compile_blueprint` + `save_assets`。

- [ ] **Step 4：`DA_ArenaWaves` 3 种混编**（MCP `ObjectTools.set_properties`，嵌套 struct 数组用转义 JSON，类引用 `{refPath:.../BP_X_C}`）

- 第 1 波：普通 ×5。
- 第 2 波：普通 ×6 + 快速 ×3。
- 第 3 波：普通 ×6 + 快速 ×4 + 坦克 ×2。
- `save_assets`。

- [ ] **Step 5：MCP/PIE 完整闭环验证（主会话+用户）**

- `StartPIE`：三波混编从四周刷出；三种敌人**血量/移速/前摇时长/伤害肉眼可分**（坦克慢且前摇长好躲、快速急且前摇短难躲）。
- 玩家能砍杀、被围不躲会死、空格闪避能穿挥击；撑过三波弹胜利、血空弹失败、重开生效（复用 M2 结算屏）。
- `GetLogEntries` 无报错。

- [ ] **Step 6：提交资产改动**

```bash
git add Content
git commit -m "feat(arena): 新增快速/坦克敌人 BP + 普通调参，DA_ArenaWaves 三种混编递增"
```

---

## Self-Review

**Spec 覆盖核对：**
- §2 敌人攻击状态机 → Task 2（纯逻辑）+ Task 4（接入）。✅
- §3 闪避翻滚 + 无敌帧（方向/无敌/去伤害/输入/收口）→ Task 3（tag+ExecCalc 收口）+ Task 5（GA_Dodge+AricHero）。✅
- §4 三种敌人数据驱动 → Task 6。✅
- §5 修双重扣防御 + 接触伤害删除 → Task 1（bug）+ Task 4（删接触）。✅
- §6 前摇视觉提示（最小、code-driven）→ Task 4 scale-pulse；受击/闪避 VFX 明确留 M5（不在计划内，符合 spec）。✅
- §8 验证策略（Automation 纯逻辑 + MCP/PIE 集成）→ 各 Task 步骤已落。✅

**占位符扫描：** 无 TBD/TODO；每个代码步骤给了完整代码；前摇手段已定为 scale-pulse（非占位）。冷却 GE 标注为可选编辑器步骤（非阻塞）。✅

**类型一致性：** `ComputeMitigatedDamage`/`StepEnemyAttack`/`FEnemyAttackStep`/`EEnemyAttackPhase`/`ShouldEnemyStrikeConnect`/`State_Invincible`/`Ability_Movement_Aric_Dodge`/`MeleeDamageEffect`/`ApplyMeleeDamageTo`/`DodgeAction`/`OnDodgeInput`/`UGA_Dodge` 在定义任务与消费任务间命名一致。✅

---

## 完成定义

开 PIE → 3 种敌人从四周涌来、贴近停下亮前摇（放大）→ 不躲被挥击掉一大块血 → 空格朝任意 WASD 方向翻滚、翻滚帧穿挥击不掉血 → 普通/快速/坦克数值差异肉眼可分 → 撑过三波胜利 / 血空失败 / 重开生效。Automation：`Valoris.ArenaCombat.ComputeMitigatedDamage`/`StepEnemyAttack`/`ShouldEnemyStrikeConnect` 三测全绿。
