# M1：竞技场核心循环（顶下操控骑士砍倒会追你的假人）实现计划

> **执行方式**：协作式迭代。每个任务 = AI 改 C++ → 用户在编辑器做配置步骤 → `Build.bat` 编译 → 用户 Play 验证检查项 → 提交。验证靠"编译通过 + 在编辑器 Play 亲眼看"，不是单元测试（UE 玩法代码的现实）。

**目标**：把 Valoris 从「RTS 相机 + AI 控英雄」改成「玩家直接 possess 骑士 + 顶下相机 + WASD 移动 + 鼠标朝向 + 左键挥砍」，并让几个假人敌人追向玩家、可被砍倒。达成后即 M1 可见胜利。

**架构**：玩家直接 possess `AAricHero`（不再 possess `ValorisSpectatorPawn`）；相机用挂在骑士身上的 SpringArm+Camera（顶下固定俯角）；移动用 `AddMovementInput`（相机相对）；攻击用现有 GAS `GA_MeleeAttack`，但把"命中缓存目标"改为"挥砍瞬间前方扇形/球形扫描命中"；敌人 AI 从"沿 Spline 走向基地"改为"追向玩家"。

**技术栈**：UE 5.7.4、C++、GAS、Enhanced Input、CharacterMovement。

## 🔖 当前进度（接力点 / 2026-06-08）

- **分支：`arena-m1`**（已建，在此分支开发；不要在 master 上改）。
- **T1 已完成并提交** ✅：`AricHero` 顶下相机(LoL 式斜俯视，CameraBoom 俯角 `-55°`/距离 `1100`，可调) + 禁用 AI possess；`ValorisGameMode` 默认 Pawn 改 `AAricHero`；编辑器侧建好 `BP_ArenaGameMode` + GameMode Override + PlayerStart，Play 验证通过（顶下视角、玩家 possess 骑士）。
- **已知资产**：`BP_Aric` 路径 `/Game/Blueprints/Heroes/Aric/BP_Aric`，父类 `AricHero`（已确认）。
- **T2(WASD 移动) 已完成并验证通过** ✅：`AricHero` 加 `SetupPlayerInputComponent`+`OnMoveInput`+`MoveAction`；编辑器侧 `IMC_Valoris` 里加了 `IA_Move`(Axis2D) 绑 WASD（W=Swizzle YXZ、S=Swizzle+Negate、A=Negate、D=无）、`BP_Aric` 的 `MoveAction` 指 `IA_Move`。Play 验证 WASD 能驱动骑士。
  - **踩坑记录（根因）**：T1 把 `PlayerControllerClass` 设成了 C++ 的 `ValorisPlayerController`，导致 `DefaultMappingContext` 为 null、`IMC_Valoris` 没加载、所有 Enhanced Input 失效（WASD 静默无反应）。修复：`BP_ArenaGameMode` 的 `PlayerControllerClass` 改用 **`BP_ValorisPlayerController`**（IMC 等资产引用设在 BP 上）。教训：UE 里资产引用走 BP/数据层，逻辑走 C++；`PlayerControllerClass`/`DefaultPawnClass` 一律指 BP 版。
  - **Live Coding 注意**：新增 `UPROPERTY`/新增成员函数这类反射改动，Live Coding 编不进去（静默不生效）；必须关编辑器用 `Build.bat` 完整重编 + 重开编辑器。
- **下一步 = T3(鼠标朝向)**：构造里 `bOrientRotationToMovement` 改回 `false`、开 Tick，Tick 里 `GetHitResultUnderCursor` 让骑士面向鼠标地面点（RInterpTo 平滑）。纯 C++，无新增编辑器资产，编译后直接 Play 验证。
- 之后 T4(左键挥砍+前方球形扫描) → T5(敌人追玩家+简易生成 = M1 验收)。
- **输入归属约定**：操控骑士的输入（Move/Attack/技能）放 `AricHero`；玩家层输入（UI/暂停/未来升级选择）放 PC。`IMC_Valoris` 是玩家级统一一份。PC 里旧 RTS 输入（右键指挥/相机缩放/建造）possess 骑士后失效，M1 暂留不清理，后续里程碑专门清。
- **待用户提供**：竞技场关卡名/路径。
- **MCP**：暂不装（选了方案 B：先推进度，少摩擦），等 UE 5.8 官方 MCP 出来再评估（免费 `github.com/remiphilippe/mcp-unreal` 是社区备选，需 Go+插件重编+会话重启）。
- 视角风格已确认：**英雄联盟那种斜俯视(顶下固定俯角)**，C++ 已实现，数值可微调。

---

> 注：本计划只覆盖 M1。M2（波次环绕生成+胜负+重开）、M3（敌人反击+闪避）、M4（三选一升级）等后续里程碑各自单独排计划，先拿下 M1 的可见胜利。

---

## 编译命令（每个任务编译时统一用）

```
& "G:\Unreal Engines\UE_5.7\Engine\Build\BatchFiles\Build.bat" ValorisEditor Win64 Development -project="F:\Valoris\Valoris.uproject" -waitmutex
```
预期结尾：`Result: Succeeded`。编译后若编辑器开着，需在编辑器里热重载或重启编辑器使改动生效。

---

## 前置准备（任务 0）

- [ ] **0.1** 建立 git 分支备份当前塔防状态，M1 在新分支开发：
  ```
  git -C F:\Valoris checkout -b arena-m1
  ```
- [ ] **0.2** 用户在编辑器侧确认现有资产名（AI 读不到 .uasset，需用户报给 AI）：
  - 玩家骑士蓝图名（推测 `BP_Aric`，在 `Content/Blueprints/Heroes/` 或 `Characters/`）
  - 是否已有 GameMode 蓝图；当前测试关卡 Map 名与路径
  - 现有 Enhanced Input 资产：`IMC_*`（映射上下文）、`IA_*`（各 InputAction）所在目录
- [ ] **0.3** 确认/准备一个竞技场测试关卡：可先用现有测试 Map，或新建一个带地面+四周墙的小场景（M1 用简单几何体即可，漂亮环境留到 M5）。

---

## 任务 1：顶下相机 + 玩家直接 possess 骑士

**Files:**
- Modify: `Source/Valoris/Character/AricHero.h`
- Modify: `Source/Valoris/Character/AricHero.cpp`
- Modify: `Source/Valoris/Core/ValorisGameMode.cpp:16`（DefaultPawnClass）

- [ ] **1.1 给骑士加顶下相机、禁用 AI possess（C++）**

`AricHero.h`：加成员
```cpp
// 顶下相机
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
TObjectPtr<class USpringArmComponent> CameraBoom;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
TObjectPtr<class UCameraComponent> TopDownCamera;
```

`AricHero.cpp`：
```cpp
#include "AricHero.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AAricHero::AAricHero()
{
    // 玩家直接控制，不再用 HeroAIController 自动 possess
    AutoPossessAI = EAutoPossessAI::Disabled;
    AIControllerClass = nullptr;

    // 顶下相机：SpringArm 固定俯角，不随角色旋转
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 1100.f;
    CameraBoom->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f));
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritYaw = false;
    CameraBoom->bInheritRoll = false;
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bUsePawnControlRotation = false;

    TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCamera->SetupAttachment(CameraBoom);
    TopDownCamera->bUsePawnControlRotation = false;

    // 角色不随控制器 yaw 转（朝向后续由移动/鼠标决定）
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->bOrientRotationToMovement = true; // M1 先朝移动方向，任务3换成朝鼠标
        Move->RotationRate = FRotator(0.f, 720.f, 0.f);
    }
}
```

- [ ] **1.2 GameMode 默认 Pawn 改为骑士（C++）**

`ValorisGameMode.cpp` 构造函数：把 `DefaultPawnClass = AValorisSpectatorPawn::StaticClass();` 改为
```cpp
// M1：玩家直接控制骑士（具体用带网格/技能配置的 BP_Aric，在 BP GameMode 里覆盖）
DefaultPawnClass = AAricHero::StaticClass();
```
并 `#include "../Character/AricHero.h"`。

- [ ] **1.3 编辑器配置**
  - 新建 `BP_ArenaGameMode`（父类 `ValorisGameMode`），`DefaultPawnClass = BP_Aric`（带网格+ABP+技能+属性的那个），`PlayerControllerClass = BP_ValorisPlayerController`（**必须用 BP 版，不是 C++ 的 `ValorisPlayerController`**——`DefaultMappingContext=IMC_Valoris` 等资产引用设在 BP 上，用 C++ 类会丢失 IMC 导致所有 Enhanced Input 失效）。
  - 竞技场关卡 World Settings → GameMode Override = `BP_ArenaGameMode`。
  - 关卡里放一个 `PlayerStart`。
  - 暂时移除场景中手动摆放的 `BP_Aric` 实例（避免和 GameMode 生成的玩家骑士重复）。

- [ ] **1.4 编译 + Play 验证**
  - 编译（见上）。
  - Play：镜头应为**顶下俯视、跟着骑士**；骑士站在 PlayerStart；不再是 RTS 边缘平移相机。

- [ ] **1.5 提交**
  ```
  git -C F:\Valoris add -A
  git -C F:\Valoris commit -m "feat(arena): 玩家直接possess骑士+顶下相机，GameMode默认Pawn改为骑士"
  ```

---

## 任务 2：WASD 移动

**Files:**
- Modify: `Source/Valoris/Character/AricHero.h` / `.cpp`（加移动输入处理）
- 编辑器：新增 `IA_Move`（Axis2D）并加入移动映射上下文

- [ ] **2.1 编辑器：输入资产**
  - 新建 `IA_Move`（Value Type = Axis2D / Vector2D）。
  - 在玩家用的 `IMC`（映射上下文）里给 `IA_Move` 绑 WASD（W=+Y, S=-Y, A=-X, D=+X，用 Modifier：Negate / Swizzle 配 2D）。
  - 确认该 IMC 在 `ValorisPlayerController::BeginPlay` 里被 Add（现有 `DefaultMappingContext`）。M1 玩家由骑士 possess，输入绑定放骑士的 `SetupPlayerInputComponent` 最直接。

- [ ] **2.2 骑士接收移动输入（C++）**

`AricHero.h`：
```cpp
protected:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    void OnMoveInput(const struct FInputActionValue& Value);

    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TObjectPtr<class UInputAction> MoveAction;
```

`AricHero.cpp`：
```cpp
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

void AAricHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAricHero::OnMoveInput);
        }
    }
}

void AAricHero::OnMoveInput(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    if (Axis.IsNearlyZero()) return;

    // 顶下固定视角：用世界轴即可（相机 yaw 固定为 0）
    AddMovementInput(FVector::ForwardVector, Axis.Y);
    AddMovementInput(FVector::RightVector,   Axis.X);
}
```
（说明：相机 yaw 固定为 0，所以世界 X=前、Y=右；若后续相机有偏转，再改成按相机 yaw 旋转输入向量。）

- [ ] **2.3 编辑器**：把 `BP_Aric` 的 `MoveAction` 指到 `IA_Move`。

- [ ] **2.4 编译 + Play 验证**：WASD 能移动骑士，移动时朝向转向移动方向，walk/run 动画播放（ABP_Aric 的 Locomotion 已做好）。

- [ ] **2.5 提交**
  ```
  git -C F:\Valoris add -A
  git -C F:\Valoris commit -m "feat(arena): WASD 直接移动骑士"
  ```

---

## 任务 3：鼠标朝向（角色面向鼠标）

**Files:** Modify `AricHero.h` / `.cpp`（Tick 里朝向鼠标）

- [ ] **3.1 C++：面向鼠标地面点**

构造函数里把 `bOrientRotationToMovement` 改回 `false`（朝向改由鼠标决定）；开启 Tick。

`AricHero.cpp`：
```cpp
// 构造函数内
PrimaryActorTick.bCanEverTick = true;
GetCharacterMovement()->bOrientRotationToMovement = false;

void AAricHero::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;

    FHitResult Hit;
    if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        FVector ToCursor = Hit.Location - GetActorLocation();
        ToCursor.Z = 0.f;
        if (!ToCursor.IsNearlyZero())
        {
            const FRotator Target(0.f, ToCursor.Rotation().Yaw, 0.f);
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), Target, DeltaTime, 15.f));
        }
    }
}
```
（`Tick` 需在 .h 声明 `virtual void Tick(float) override;`）

- [ ] **3.2 编译 + Play 验证**：骑士始终面向鼠标光标方向；WASD 移动时可"边走边瞄"（移动与朝向解耦）。

- [ ] **3.3 提交**
  ```
  git -C F:\Valoris add -A
  git -C F:\Valoris commit -m "feat(arena): 角色面向鼠标朝向"
  ```

---

## 任务 4：左键挥砍 + 前方命中扫描

**Files:**
- Modify: `AricHero.h` / `.cpp`（绑左键 → 激活近战技能）
- Modify: `Source/Valoris/GAS/Abilities/Melee/GA_MeleeAttack.h` / `.cpp`（命中改为前方扫描）

- [ ] **4.1 左键激活近战技能（C++）**

`AricHero.h`：加 `UPROPERTY(EditDefaultsOnly,Category="Input") TObjectPtr<UInputAction> AttackAction;` 和 `void OnAttackInput();`

`AricHero.cpp`：`SetupPlayerInputComponent` 内加
```cpp
if (AttackAction)
{
    EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &AAricHero::OnAttackInput);
}
```
```cpp
#include "AbilitySystemComponent.h"
#include "../GAS/ValorisGameplayTags.h"

void AAricHero::OnAttackInput()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        FGameplayTagContainer Tags;
        Tags.AddTag(FValorisGameplayTags::Ability_Melee_Single_Aric_Attack);
        ASC->TryActivateAbilitiesByTag(Tags);
    }
}
```

- [ ] **4.2 近战命中改为前方扫描（C++，修改 `GA_MeleeAttack`）**

现状：`OnEventReceived` 在收到 `Event.Attack.Hit` 时对**缓存目标**造成伤害（RTS 自动锁定遗留）。玩家挥砍没有预设目标，改为**在命中事件瞬间，对角色前方一定范围内的敌人做球形扫描并逐个造成伤害**。

`GA_MeleeAttack.cpp` 的 `OnEventReceived` 改为：
```cpp
void UGA_MeleeAttack::OnEventReceived(FGameplayEventData Payload)
{
    if (!Payload.EventTag.MatchesTag(FValorisGameplayTags::Event_Attack_Hit))
    {
        return;
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar) return;

    const FVector Origin = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * (AttackRange * 0.5f);
    TArray<FHitResult> Hits;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRange * 0.5f);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Avatar);

    Avatar->GetWorld()->SweepMultiByChannel(
        Hits, Origin, Origin, FQuat::Identity, ECC_Pawn, Sphere, Params);

    TSet<AActor*> Damaged;
    for (const FHitResult& Hit : Hits)
    {
        AActor* Target = Hit.GetActor();
        if (Target && !Damaged.Contains(Target)
            && UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
        {
            ApplyDamageToTarget(Target);
            Damaged.Add(Target);
        }
    }
}
```
（`ApplyDamageToTarget` 保持不变；保留旧的缓存目标逻辑也行，但玩家流程走扫描这条。）

- [ ] **4.3 编辑器**：把 `BP_Aric` 的 `AttackAction` 指到左键 `IA_Attack`（若没有则新建并加进 IMC 绑左键）。确认 `BP_Aric` 的 `DefaultAbilities` 里包含 Aric 普攻技能蓝图，且其 AbilityTags 含 `Ability.Melee.Single.Aric.Attack`、攻击 Montage 里有发送 `Event.Attack.Hit` 的 AnimNotify（这些上一阶段已配好）。

- [ ] **4.4 编译 + Play 验证**：左键播放挥砍动画；站在敌人前方左键 → 前方敌人扣血（敌人头顶血条下降）。

- [ ] **4.5 提交**
  ```
  git -C F:\Valoris add -A
  git -C F:\Valoris commit -m "feat(arena): 左键挥砍，命中改为前方球形扫描"
  ```

---

## 任务 5：假人敌人追向玩家 + 简易生成（M1 收尾）

**Files:**
- Modify: `Source/Valoris/Enemy/EnemyBase.h` / `.cpp`（Tick 改为追玩家）
- Modify: `Source/Valoris/Core/ValorisGameMode.cpp`（M1 简易生成，绕开 Spline/WaveData）

- [ ] **5.1 敌人追向玩家（C++）**

`EnemyBase.cpp` 的 `Tick`：把"沿 Spline 移动"替换为"朝玩家移动"（M1 不依赖 NavMesh，直接 `AddMovementInput` 朝玩家；平地竞技场足够）：
```cpp
void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;

    FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    ToPlayer.Z = 0.f;
    const float Dist = ToPlayer.Size();
    if (Dist > 120.f) // 离玩家还远就继续靠近（120 内算贴身，M1 不攻击只贴着）
    {
        const FVector Dir = ToPlayer.GetSafeNormal();
        AddMovementInput(Dir, 1.f);
        SetActorRotation(FRotator(0.f, Dir.Rotation().Yaw, 0.f));
    }
}
```
需要 `#include "Kismet/GameplayStatics.h"`，并确认 `EnemyBase` 构造里 `PrimaryActorTick.bCanEverTick = true;`（基类 Tick 默认关，需在 EnemyBase 打开）。SetPath/Spline 相关 M1 可不调用。

- [ ] **5.2 M1 简易生成（C++）**

`ValorisGameMode.cpp` 的 `BeginPlay`：M1 阶段**不**调用塔防的 `StartWaves()`（它需要 WaveData/EnemyPath，会告警/无敌人）。改为生成几个测试敌人围在玩家四周：
```cpp
// 替换 BeginPlay 末尾的 StartWaves(); 调用（M1 临时）
void AValorisGameMode::BeginPlay()
{
    Super::BeginPlay();
    BaseHealth = BaseMaxHealth;

    // M1：在玩家四周生成几个测试假人（M2 接回正式波次系统）
    if (M1TestEnemyClass)
    {
        if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
        {
            const FVector Center = Player->GetActorLocation();
            for (int32 i = 0; i < 5; ++i)
            {
                const float Ang = i * (360.f / 5.f);
                const FVector Pos = Center + FRotator(0,Ang,0).Vector() * 800.f + FVector(0,0,100.f);
                FActorSpawnParameters P; P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
                GetWorld()->SpawnActor<AEnemyBase>(M1TestEnemyClass, Pos, FRotator::ZeroRotator, P);
            }
        }
    }
}
```
`ValorisGameMode.h` 加：
```cpp
UPROPERTY(EditDefaultsOnly, Category = "M1")
TSubclassOf<class AEnemyBase> M1TestEnemyClass;
```
（玩家 BeginPlay 与 GameMode BeginPlay 时序：若取不到 Player，可改用一个 0.5s 延迟 timer 再生成；验证时若没刷出来就用此法兜底。）

- [ ] **5.3 编辑器**：`BP_ArenaGameMode` 的 `M1TestEnemyClass` 指向一个敌人蓝图（用现有 `BP_Enemy*`，确保它有网格+ABP_Enemy 动画+属性 GE 配置，会动会有血条）。

- [ ] **5.4 编译 + Play 验证（M1 验收）**：
  - Play → 顶下视角，骑士在场中，四周刷出 5 个假人。
  - 假人朝玩家走过来。
  - WASD 走位、鼠标瞄准、左键把假人一个个砍掉（血条归零后死亡/消失）。
  - **以上全做到 = M1 达成（骑士在竞技场砍倒会追你的假人）。**

- [ ] **5.5 提交**
  ```
  git -C F:\Valoris add -A
  git -C F:\Valoris commit -m "feat(arena): 敌人追向玩家 + M1 简易生成，达成竞技场核心循环雏形"
  ```

---

## M1 完成定义

顶下视角下，玩家用 WASD + 鼠标朝向 + 左键，在竞技场里砍倒若干会主动追向自己的假人敌人——可见、可玩、能跑。达成后进入 M2（正式波次环绕生成 + 玩家血量/胜负 + 重开）单独排计划。

## M1 风险/兜底

- **possess/相机不对**：检查 `BP_ArenaGameMode` 的 DefaultPawnClass、地图 GameMode Override、是否残留手摆的 BP_Aric。
- **左键不触发技能**：确认普攻技能 AbilityTags 含 `Ability.Melee.Single.Aric.Attack`，且已在 BP_Aric 的 DefaultAbilities 授予。
- **敌人不动**：确认 EnemyBase Tick 开启、敌人蓝图基于会动的 BP_Enemy。
- **生成时序取不到 Player**：用延迟 timer 兜底（见 5.2 备注）。
