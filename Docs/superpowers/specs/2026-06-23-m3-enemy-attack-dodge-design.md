# M3 设计：敌人主动攻击 + 闪避无敌帧 + 3 种敌人

| 项目 | 内容 |
|------|------|
| 里程碑 | M3（v1 第三个里程碑，承接已完成并合并 master 的 M1/M2） |
| 引擎 | Unreal Engine 5.8（C++ + GAS + Enhanced Input） |
| 目标 | 把 M2 的「贴身持续掉血」升级为「敌人起手前摇、挥击一次性结算的真攻击」，玩家空格翻滚带无敌帧可完整躲掉，并上 3 种数值不同的敌人 |
| 上位文档 | `Docs/superpowers/specs/2026-06-08-arena-roguelite-v1-design.md`（v1 总设计，第 15 节里程碑表 M3 = 敌人会反击 + 闪避 + 2~3 敌种） |
| 前序 | M2 `Docs/superpowers/specs/2026-06-21-m2-arena-waves-survival-design.md`（已完成并合并 master，merge `d80923f`） |

> 本文档只覆盖 M3。M4（三选一升级+UI+强化池）、M5（打磨/小Boss/特效音效/数值平衡）、M6（打包上线）各自单独排计划。塔系统清理为独立里程碑。

---

## 1. 范围与完成定义

**一句话**：敌人靠近后停下、起手前摇（telegraph）、挥击一次性结算伤害；玩家按空格朝当前移动方向翻滚、翻滚期间无敌可完整躲掉这一击；3 种数值不同的敌人（普通/快速/坦克）混编进波次。

**M3 做**：
- 敌人**离散前摇攻击**替换 M2 的贴身持续接触掉血（动作品类的"真攻击"，可被闪避躲掉）。
- 空格**闪避翻滚 + 无敌帧**：朝 WASD 移动输入方向翻滚，无敌帧期间免疫一切伤害。
- **3 种敌人**（普通/快速/坦克），仅数值差异，共用一套攻击逻辑，数据驱动。
- 修复既有**双重扣防御 bug**（伤害计算收口到唯一处）。

**M3 完成定义（验收）**：
开 PIE → 3 种敌人从四周涌来、贴近后**停下亮起前摇提示** → 不躲就被挥击扣一大块血 → **空格朝任意 WASD 方向翻滚、翻滚帧穿过挥击不掉血** → 普通/快速/坦克在血量·速度·前摇·伤害上明显不同 → 撑过波次胜利 / 血空失败（复用 M2 胜负结算屏）。

**M3 不做（明确划给后续里程碑）**：
- Q/E/R 主动技能（盾击/旋风斩/战吼）→ 后续。
- 受击/死亡演出动画、敌人攻击 Montage、闪避 VFX → **M5 打磨**。
- NavMesh / 行为树 AI（平地竞技场 Tick 直驱够用）。
- 坦克等"独特招式"（三种共用同一套挥击逻辑，仅数值不同）。
- 数值平衡、小 Boss → **M5**。
- 塔系统遗留清理（`Tower/`/`BP_TowerBase`/`EnemyPath`/经济/Spline/RTS 输入）→ **独立清理里程碑**。

---

## 2. 敌人攻击状态机（扩展 `EnemyBase::Tick`，方案 A1）

### 2.1 为什么放 Tick 而非 AIController
- M2 已把"追玩家"移动逻辑放在 `EnemyBase::Tick`（平地竞技场不依赖 NavMesh，直接 `AddMovementInput`）；攻击状态机紧贴这套移动，放 Tick 改动集中、与现状一致。
- 空壳 `EnemyAIController` 的"正统化"（行为树/NavMesh）留到真有需求时再做，不在 M3 扩散。

### 2.2 状态机
状态枚举 `EEnemyAttackPhase`：`Approaching` / `WindingUp` / `Recovering`（`Strike` 是 WindingUp 计时到点的瞬时事件，不是常驻相位）。

```
Approaching --(进入 AttackRange)--> WindingUp --(windup 计时到: Strike 命中判定)--> Recovering --(冷却到)--> Approaching
                  ^                                                                                          |
                  +------------------------------------(玩家跑出 range，前摇被打断回退)---------------------+
```

- **Approaching**：保留 M2 现有"朝玩家移动 + 朝向"逻辑。到玩家距离 ≤ `AttackRange`（建议 ~150，比 M2 贴身 120 略大）→ 转 `WindingUp`，停止移动。
- **WindingUp**：累加 `WindupAccumulator`，期间播**简易视觉提示**（见 §6）。
  - 计时达 `WindupDuration` → 触发 **Strike**。
  - 若期间玩家离开 range 超过 `AttackRange`（被走位/闪避骗招）→ 回退 `Approaching`，重置累加器（让走位有意义）。
- **Strike（瞬时）**：做一次前方命中判定 —— 玩家在 `AttackReach` 内且**未带 `State.Invincible`** → 施加 `GE_Damage`（SetByCaller `AttackDamage`）；否则**挥空**（玩家躲掉）。随后转 `Recovering`。
- **Recovering**：累加 `RecoveryAccumulator`，达 `RecoveryDuration` → 回 `Approaching`（玩家仍在 range 内则会重新进入下一轮前摇）。

### 2.3 纯逻辑抽测（TDD）
- 相位推进抽成纯函数（签名以实现计划为准，行为约定）：给定当前相位、累加器、DeltaTime、`WindupDuration`、`RecoveryDuration`、是否在攻击距离内 → 返回下一相位 + 是否在本帧触发 Strike + 是否重置累加器。
- 命中判定抽成纯函数：给定到玩家距离、`AttackReach`、目标是否无敌 → 返回是否结算伤害。
- 二者上 Automation 测试（先 RED 后 GREEN）。
- 施加伤害的 GE 逻辑复用 M2 `EnemyBase::ApplyContactDamageTo` 的 `MakeOutgoingSpec` + `SetByCallerMagnitude(Data_Damage, ...)` 框架，**重命名为 `ApplyMeleeDamageTo`** 并由 Strike 调用（集成路径，靠 MCP/PIE 验证，不写单测）。

---

## 3. 闪避翻滚 + 无敌帧（改造 `GA_Charge`，方案 B1）

### 3.1 重命名为 `GA_Dodge`
- `GA_Charge` 被完全改用途为闪避，且旧 Charge 当前未绑输入、未在使用（死代码）。按"删除而非注释 / 复用优先 / 命名表义"规范，**重命名类 `GA_Charge → GA_Dodge`**，并把对应 tag `Ability.Movement.Aric.Charge → Ability.Movement.Aric.Dodge`。

### 3.2 三处改造
1. **方向**：取 `AricHero` 当前移动输入向量（最近一帧 WASD 合成方向）；零输入时回退 actor forward。不再用 `GA_Charge` 原来的 forward / TriggerEventData target。
2. **无敌帧**：翻滚激活时给自身 ASC 挂**新增 `State.Invincible` 标签**，翻滚结束移除。
3. **去伤害**：删除撞击伤害整套（`ImpactDamageEffect` / `CheckImpact` / `ApplyImpactDamage` / `HitTargets` / `ImpactRadius` / `DamageMultiplier`）。闪避是纯位移 + 免伤，不打人。
- 复用现有定时器位移框架（`PerformCharge` 那套高频定时器平滑位移），按 dodge 语义调参（距离/速度，具体数值 M5 平衡）。

### 3.3 输入与冷却
- `AricHero` 加**空格**输入绑定 + 在角色初始化时**授予 `GA_Dodge`**。
- **冷却**：复用现有冷却机制（`GE_Cooldown` / ability cooldown），短冷却（~0.8s）防连滚。

### 3.4 无敌帧落地点（伤害收口）
- 在 `ExecCalc_Damage::Execute_Implementation` 开头早退：目标带 `State.Invincible` 标签 → `FinalDamage = 0`（不输出修改器）。
- 单一收口，对接触/挥击/任何 `GE_Damage` 来源都生效；目标标签从 `EvaluateParams.TargetTags` 读取。

---

## 4. 三种敌人（数据驱动，不加 C++ 子类）

一套 `EnemyBase` + `EditDefaultsOnly` 参数，派生 3 个蓝图调参，不为每种敌人写 C++ 子类。

| 敌种 | 血量 | 移速 | 攻击伤害 | 前摇 `WindupDuration` | 定位 |
|------|------|------|----------|------------------------|------|
| 普通 EnemyNormal | 中 | 中 | 中 | 中（~0.4s） | 基准 |
| 快速 EnemyFast | 低 | 高 | 低~中 | 短（~0.25s） | 快但脆、前摇短难躲 |
| 坦克 EnemyTank | 高 | 低 | 高 | 长（~0.7s） | 慢但痛、前摇长好躲 |

- 参数化字段（`EditDefaultsOnly`）：`AttackRange` / `AttackReach` / `AttackDamage` / `WindupDuration` / `RecoveryDuration`；血量经属性初始化、移速经 `CharacterMovement.MaxWalkSpeed`。
- `DA_ArenaWaves` 更新为 3 种混编递增（前期普通为主，逐波掺快速/坦克）。
- 3 个 BP 用 MCP/编辑器配置（BP CDO 改默认值后须 `compile_blueprint` 才被 PIE 认，见项目 MCP 工作流）。

---

## 5. 修复双重扣防御 bug

- 现状：`ExecCalc_Damage` 算 `max(1, Dmg - Def)`，而 `ValorisAttributeSet::PostGameplayEffectExecute` **又**做 `ActualDamage = max(Dmg - Def, 0)` —— 防御被减两次（既有 bug，非 M2 引入，影响近战/接触/挥击所有伤害）。
- 修复：**删除** `PostGameplayEffectExecute` 里重复的那次减防御，伤害计算**唯一收口在 `ExecCalc_Damage`**。`PostGameplayEffectExecute` 仅负责把已算好的 `IncomingDamage` 扣到 `Health` + 死亡判定。
- 给敌人 `AttackDamage` 合理 C++ 默认值（> 典型防御，避免恒为最低 1），各 BP 按 §4 覆盖。
- M2 接触伤害整套（`ContactDamageEffect` / `ContactDamage` / `ContactDamageInterval` / `ContactDamageAccumulator` / Tick 贴身分支）随攻击模型替换一并删除；`ArenaCombat::AdvanceContactDamageTimer` 及其 Automation 测试若不再被任何代码引用则删除（重构清理规范，删前 Grep 确认无引用）。

---

## 6. 反馈与表现（最小）

- **敌人前摇视觉提示**：先用最省的一种实现（材质变红 / 轻微放大 / 脚下 decal 三选一，实现计划阶段定具体手段），确保玩家能读出"它要打了"。
- **玩家受击**：复用 M2 已有 HUD 血条反映离散掉血；**M3 不加受击动画**。可选极简受击屏闪（省事则做，否则留 M5）。
- **闪避表现**：M3 不强加 VFX/拖影，留 M5。

---

## 7. 复用 / 改造 / 新写 / 删除 总览

| 类别 | 内容 |
|------|------|
| ♻️ 复用 | `EnemyBase` Tick 追玩家、`GE_Damage` / `ExecCalc_Damage`、GE 施加框架（MakeOutgoingSpec+SetByCaller）、`AricHero` 输入系统、M2 HUD 血条与胜负结算屏、`DA_ArenaWaves` 结构、`GA_Charge` 定时器位移框架 |
| 🔧 改造 | `EnemyBase`（加攻击状态机 + `ApplyMeleeDamageTo`）、`GA_Charge→GA_Dodge`（方向/无敌/去伤害）、`ExecCalc_Damage`（无敌早退 + 作为唯一伤害收口）、`ValorisAttributeSet::PostGameplayEffectExecute`（删重复减防御）、`DA_ArenaWaves`（3 种混编）、`AricHero`（空格绑定 + 授予闪避）、`ValorisGameplayTags`（Charge→Dodge tag、加 Invincible） |
| 🆕 新写 | `State.Invincible` tag、`EEnemyAttackPhase` 枚举、攻击相位推进 + 命中判定纯函数及 Automation 测试、3 个敌人 BP（EnemyNormal/Fast/Tank）、敌人前摇视觉提示 |
| 🗑️ 删除 | M2 接触伤害整套、`PostGameplayEffectExecute` 重复扣防御段、`GA_Charge` 撞击伤害逻辑、（若无引用）`AdvanceContactDamageTimer` 纯函数及其测试 |

---

## 8. 验证策略

- **Automation TDD（纯逻辑）**：① 攻击相位推进（Approaching→WindingUp→Strike→Recovering→循环、前摇被打断回退）；② 命中判定（距离内/外、目标无敌时不结算）；③ 修复后伤害公式（防御只减一次）。先 RED 后 GREEN。
- **MCP / PIE（集成）**：
  - spawn 3 种敌人，观察 Approaching→停下→前摇提示→Strike 的完整相位。
  - GAS Inspector 读玩家 `Health`：不躲被挥击 → 掉一大块；翻滚帧穿过挥击 → 血不变（验无敌帧）。
  - 3 种敌人血量/移速/伤害/前摇差异可观测。
  - 完整闭环：3 种混编波次能打、能胜、能死、能重开（复用 M2）。
- **编译命令**：`& "G:\Unreal Engines\UE_5.8\Engine\Build\BatchFiles\Build.bat" ValorisEditor Win64 Development -project="F:\Valoris\Valoris.uproject" -waitmutex`。
- **执行方式**：subagent-driven 子代理逐任务（不命名 + 后台 + opus），UE 现实混合流（子代理写 C++/测试/提交；编译 + MCP 跑测试/PIE/读 GAS + 用户编辑器配置由主会话/用户做）；每任务后两轮独立审查（spec=general-purpose、代码质量=`pr-review-toolkit:code-reviewer`）。

---

## 9. 风险与应对

| 风险 | 应对 |
|------|------|
| 攻击状态机塞进 Tick 变臃肿 | 相位推进/命中判定抽纯函数（可测、Tick 只做状态持有与副作用），逻辑外移 |
| `GA_Charge→GA_Dodge` 重命名牵连 tag/引用编译报错 | 重命名前 Grep 全量排查 `Charge` 引用（tag、授予处、BP 引用）；类与 tag 同步改 |
| 无敌帧没生效（标签没挂上/没读到） | 收口在 `ExecCalc_Damage` 单点早退；MCP 读玩家标签 + 翻滚穿挥击血量自查 |
| 删重复减防御误伤近战数值 | 修复后用 Automation 验伤害公式；MCP/PIE 验近战砍敌仍正常掉血 |
| 敌人移动输入方向取不到（闪避方向错） | `AricHero` 缓存最近移动输入向量供 `GA_Dodge` 读取；零输入回退 forward |
| 范围蔓延（弃坑主因） | 严守第 1 节边界；Q/E/R、动画演出、平衡一律记 M5/后续，不进 M3 |

---

## 10. 里程碑完成后

M3 达成 = v1 的战斗循环完整（敌人会反击、玩家能闪避走位、有敌种区分）。下一步 **M4**（局内三选一升级 + UI + 强化池），单独 brainstorm + 排计划。M2 预留的波间窗口（`OnWaveCompleted` 后、`StartNextWave` 前）即 M4 三选一接入点。
