# M2 设计：竞技场波次生存（环绕生成 + 玩家可死亡 + 胜负 + 重开）

| 项目 | 内容 |
|------|------|
| 里程碑 | M2（v1 第二个里程碑，承接已完成的 M1 核心循环） |
| 引擎 | Unreal Engine 5.8（C++ + GAS + Enhanced Input） |
| 目标 | 把 M1 的「砍会追你的假人」雏形，补成「一波波打、能赢能死能重开」的完整循环骨架 |
| 上位文档 | `Docs/superpowers/specs/2026-06-08-arena-roguelite-v1-design.md`（v1 总设计） |
| 前序 | `Docs/superpowers/plans/2026-06-08-m1-arena-core-loop.md`（M1 已完成并合并 master） |

> 本文档只覆盖 M2。M3（敌人主动攻击+闪避+多敌种）、M4（三选一升级+UI+强化池）、M5（打磨/平衡/小Boss）、M6（打包上线）各自单独排计划。

---

## 1. 范围与完成定义

**一句话**：玩家在竞技场里抵挡若干波从四周涌来的敌人，被围会掉血、血空则失败，撑过第 N 波则胜利，结算屏可一键重开。

**M2 做**：
- 波次环形生成 + 递增（数据驱动，复用 `UWaveData`）。
- 敌人**贴身接触**对玩家掉血（survivors-like 品类标准模型，非临时方案）。
- 玩家血量归零 = 失败；撑过第 N 波 = 胜利。
- 极简结算屏（胜利/失败 + 撑过 X 波）+「重开」按钮（重载当前关卡）。
- 波间「短暂喘息 +『第 N 波』横幅，自动开下一波」，并把波间窗口做成 M4 三选一的预留插槽。
- HUD：玩家血条、当前/总波数、本波剩余敌人数、「第 N 波」横幅。

**M2 完成定义（验收）**：
开 PIE → 三波递增的假人从四周涌来 → 被围会掉血、血空弹出「失败」屏 → 或砍光三波弹出「胜利」屏 → 点「重开」重来一局。

**M2 不做（明确划给后续里程碑）**：
- 敌人主动挥击 / 带前摇可被闪避躲掉的「真攻击」、闪避无敌帧、2~3 种敌人 → **M3**。
- 三选一升级 UI + 强化池 → **M4**（M2 只预留波间插槽）。
- 玩家受击动画、击杀数/计时统计、数值平衡、小 Boss → **M3/M5**。

---

## 2. 波次系统（复用为主，改一处）

### 2.1 直接复用（不动）
- 数据结构：`UWaveData` / `FWaveInfo`（`WaveName` / `Enemies` / `DelayAfterWave`）/ `FEnemySpawnInfo`（`EnemyClass` / `Count` / `SpawnInterval`）。
- 流程：`StartWaves()` / `StartNextWave()` / `SpawnNextEnemy()`（定时器生成）/ `CheckWaveCompletion()` / `OnEnemyDestroyed()` / `AliveEnemyCount`。
- 事件：`OnWaveStarted(WaveIndex)` / `OnWaveCompleted(WaveIndex)` / `OnAllWavesCompleted` / `OnGameOver(bVictory)`。

### 2.2 新建资产
- `DA_ArenaWaves`（`UWaveData` 实例）：配 **3 波递增**。建议 第1波 5 个、第2波 8 个、第3波 12 个 `BP_EnemyBasic`，`SpawnInterval` 随波递减、`DelayAfterWave` 给 ~3s 喘息。
- **M2 先 3 波快速验证闭环；M5 平衡阶段再扩到 8~10 波**——数据驱动，扩展只改资产、不改代码。

### 2.3 唯一改造点：生成位置
- 原塔防：敌人在 `EnemyPath` Spline 起点生成、沿路径走向基地。
- M2：敌人在**玩家四周环形**生成（半径约 1200，均匀分布 + 随机角度抖动），生成后直接奔向玩家（M1 已实现敌人追玩家 Tick）。
- 复用 M1 `SpawnM1TestEnemies` 的环形定位思路，把该临时函数**删除**，逻辑并入正式 `SpawnEnemy` 的位置计算。

### 2.4 波间流程
- `BeginPlay` 改回调用 `StartWaves()`（M1 临时改成了 `SpawnM1TestEnemies`，本里程碑改回）。
- 一波清空 → `OnWaveCompleted` → 等待 `DelayAfterWave`（短暂喘息）→ `StartNextWave` → `OnWaveStarted` → HUD 弹「第 N 波」横幅。
- **M4 插槽**：`OnWaveCompleted` 触发后、`StartNextWave` 前的这段延迟窗口，就是 M4 三选一弹窗的接入点。M2 此处只做计时喘息，但代码结构要让 M4 能在此暂停流程、等玩家选完再继续。

---

## 3. 玩家受伤 / 死亡（新写，最小）

### 3.1 接触伤害
- `EnemyBase::Tick` 中 M1 已计算到玩家的 `Dist` 并在 120 内贴身停。M2 复用同一个「贴身」判定（`Dist <= 贴身距离`，即 M1 的 120，不再引入第二个阈值），在贴身状态下**按固定间隔（建议 0.5s）对玩家 ASC 施加 `GE_Damage`**。
- 复用现有 `GE_Damage` + `ExecCalc_Damage`；每个敌人各自累计接触计时，避免多敌人叠加时一帧多跳。
- 接触伤害数值用 `GE_Damage` 现有机制配置（M2 给一个能让玩家在被多个敌人围住时几秒内危险的低值，具体数值 M5 平衡）。

### 3.2 玩家血量与死亡
- 复用 `ValorisAttributeSet.Health`，**不新增属性**。
- 死亡触发：监听玩家 `Health <= 0`（镜像现有敌人死亡的属性变化回调路径）→ 通知 GameMode `OnGameOver(false)`。
- **M2 不做受击动画**（仅掉血 + 死亡判定）；受击/死亡演出留 M3。

---

## 4. 胜负 / 结算 / 重开

- **胜利**：`AreAllWavesCompleted()`（撑过第 N 波且场上清空）→ `OnGameOver(true)`。
- **失败**：玩家 `Health <= 0` → `OnGameOver(false)`。
- **新建 `WBP_GameOver`（UMG）**：显示「胜利 / 失败」+「撑过 X 波」+「重开」按钮。
  - 「重开」→ `UGameplayStatics::OpenLevel(当前关卡)`，永久死亡、不存档不解锁。
  - `OnGameOver` 触发时：显示面板、暂停游戏、解锁并显示鼠标光标。

---

## 5. HUD（改造现有 `ValorisHUD`）

- 玩家血条（绑 `ValorisAttributeSet.Health`）。
- 当前波数 / 总波数（绑 `OnWaveStarted`）。
- 本波剩余敌人数（绑 `AliveEnemyCount` / `OnEnemyDestroyed`）。
- 「第 N 波」横幅（`OnWaveStarted` 触发，短暂显示后淡出）。
- **删除** HUD 中的基地血量 / 金币显示（塔防遗留）。

---

## 6. GameMode 改造 + 本里程碑清理边界

### 6.1 M2 内删除（被直接取代的死代码，遵循「删除而非注释」规范）
- 基地血量整套：`BaseHealth` / `BaseMaxHealth` / `DamageBase` / `OnBaseHealthChanged`（胜负从基地血改为玩家死亡，这套彻底失效）。
- M1 临时生成：`M1TestEnemyClass` / `SpawnM1TestEnemies`。
- HUD 的基地血量 / 金币显示控件。

### 6.2 暂留（→ 专门的「塔系统清理」里程碑，不在 M2 扩散）
- `Tower/`、`BP_TowerBase` / `BP_GA_TowerAttack`、`GA_TowerAttack`。
- `ResourceManager`（金币经济）、`FWaveInfo.GoldReward` 字段（属经济系统，随塔清理一起删）。
- `EnemyPath` / Spline 相关、`ValorisSpectatorPawn`、PlayerController 旧 RTS 输入。

> 说明：M2 只删「被本里程碑直接取代」的代码；更大的塔防遗留系统集中到独立清理里程碑一次性删干净，避免 M2 范围蔓延。

---

## 7. 复用 / 改造 / 新写 / 删除 总览

| 类别 | 内容 |
|------|------|
| ♻️ 复用 | `UWaveData`/`FWaveInfo`/`FEnemySpawnInfo`、GameMode 波次流程与事件、`AliveEnemyCount`、`ValorisAttributeSet.Health`、`GE_Damage`/`ExecCalc_Damage`、`EnemyBase` 追玩家 Tick、`ValorisHUD` 框架 |
| 🔧 改造 | 敌人生成位置（Spline→环形）、`BeginPlay`（改回 `StartWaves`）、胜负模型（基地血→玩家死亡）、`EnemyBase::Tick`（加贴身接触伤害）、`ValorisHUD`（血条/波数/剩余敌人/横幅） |
| 🆕 新写 | `DA_ArenaWaves` 资产、玩家死亡监听→`OnGameOver`、`WBP_GameOver` 结算屏 + 重开、「第 N 波」横幅、波间 M4 插槽结构 |
| 🗑️ 删除 | 基地血量整套、`M1TestEnemyClass`/`SpawnM1TestEnemies`、HUD 基地/金币显示 |

---

## 8. 验证策略

- **AI 自助验证（MCP，UE 5.8 官方）**：spawn 敌人、跑 PIE、读玩家/敌人 ASC 血量、读 Output Log，先自查再交用户。
- **用户 Play 终验**：按第 1 节「完成定义」跑一遍完整闭环。
- **纯逻辑上 Automation 测试**（按合作模式 B「纯逻辑才上 Automation」）：接触伤害累计节奏、波次推进、胜负判定。
- **编译命令**：`& "G:\Unreal Engines\UE_5.8\Engine\Build\BatchFiles\Build.bat" ValorisEditor Win64 Development -project="F:\Valoris\Valoris.uproject" -waitmutex`。

---

## 9. 风险与应对

| 风险 | 应对 |
|------|------|
| 玩家死亡监听接错（属性回调路径与敌人不同） | 先读现有敌人死亡回调实现，镜像到玩家；MCP 读 ASC 血量自查 |
| 多敌人围身接触伤害一帧多跳 | 每敌人独立累计接触计时，按间隔施加，不按帧 |
| 删基地血量牵连 HUD/事件编译报错 | 用 Grep 全量排查 `BaseHealth`/`DamageBase`/`OnBaseHealthChanged` 引用后再删 |
| 重开后状态残留 | 重开走 `OpenLevel` 整关重载，天然清状态 |
| 范围蔓延（弃坑主因） | 严守第 1 节边界；敌人真攻击/闪避/升级 UI 一律记 M3/M4，不进 M2 |

---

## 10. 里程碑完成后

M2 达成 = v1 的完整可玩循环骨架成型（打波次、会死、能赢、能重开）。下一步 M3（敌人会反击 + 闪避无敌帧 + 2~3 种敌人），单独 brainstorm + 排计划。
