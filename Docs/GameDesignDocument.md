# Valoris - 3D塔防游戏开发文档

## 项目概述

| 项目 | 内容 |
|------|------|
| 游戏名称 | Valoris |
| 引擎版本 | Unreal Engine 5.7 |
| 开发周期 | 3个月（约90小时） |
| 技术路线 | C++ 为主，蓝图仅用于资源配置 |
| 技能系统 | Gameplay Ability System (GAS) |
| 输入系统 | Enhanced Input System |

### 核心玩法

- 玩家控制一个英雄角色，使用类似《英雄联盟》的操控方式（RTS风格镜头）
- 玩家可以建造、升级防御塔来抵御敌人波次
- 英雄可以主动战斗，拥有技能系统
- 敌人沿固定路径进攻基地，玩家需要保护基地不被摧毁

---

## 技术架构

### 代码与蓝图职责划分

**C++ 负责**：所有游戏逻辑

**蓝图负责**：
- 设置 Mesh/材质等美术资源
- 配置 DataAsset（英雄属性、塔属性、敌人属性等）
- 配置 GameplayAbility（技能参数、Montage、特效等）
- 配置 GameplayEffect（数值效果）
- UI 布局（UMG）
- 关卡设计

### 核心模块依赖

EnhancedInput, GameplayAbilities, GameplayTags, GameplayTasks, AIModule, NavigationSystem, Niagara, UMG, Slate, SlateCore

### 源码目录结构

```
Source/Valoris/
├── Core/           # GameMode, PlayerController, GameState
├── Character/      # 角色基类（带ASC）、英雄角色、移动组件
├── Input/          # 输入组件、输入配置、屏幕边缘触发器
├── Camera/         # RTS风格相机
├── Tower/          # 塔基类、塔管理器、投射物
├── Enemy/          # 敌人基类、AI控制器、生成器、波次管理
├── GAS/            # ASC、AttributeSet、GA基类、具体技能、GE效果
├── Combat/         # 目标选择
├── Building/       # 建造系统、预览、网格
├── Economy/        # 资源管理
├── Data/           # DataAsset定义
└── UI/             # HUD、各类Widget
```

---

## 核心系统设计

### 1. 相机系统（RTS风格）

| 输入 | 功能 |
|------|------|
| 屏幕边缘 | 相机平移 |
| 滚轮 | 缩放 |
| 中键拖拽 | 旋转视角 |
| 空格 | 聚焦英雄 |

相机有边界限制，不能移出地图。

### 2. 英雄系统

**操控方式**：
- 右键点击地面 → 移动到目标位置
- 右键点击敌人 → 自动攻击目标
- Q/W/E/R → 释放技能
- A键 + 左键 → 攻击移动

**属性**：生命值、法力值、移动速度、攻击力、攻速、攻击范围、护甲

### 3. 防御塔系统

**塔类型**（第一版3种）：
| 类型 | 特点 |
|------|------|
| 箭塔 | 单体伤害，攻速快 |
| 炮塔 | AOE伤害，攻速慢 |
| 减速塔 | 减速敌人 |

支持升级系统。

### 4. 敌人系统

**敌人类型**：
| 类型 | 特点 |
|------|------|
| 普通 | 标准属性 |
| 快速 | 血少速度快 |
| 坦克 | 血多速度慢 |
| 飞行 | 无视地形 |
| Boss | 特殊机制 |

敌人沿预设路径移动，到达基地造成伤害，不会还击。

### 5. 波次系统

通过 DataAsset 配置每波敌人类型、数量、生成间隔、波次奖励。

### 6. 建造系统

1. 玩家选择塔类型
2. 显示建造预览（绿色=可建造，红色=不可建造）
3. 检测资源和位置
4. 左键确认 / 右键取消
5. 扣除资源，生成塔

### 7. 经济系统

单一货币：金币
- 击杀敌人获得
- 通过波次获得
- 用于建造/升级塔

---

## GAS 设计

### AttributeSet 属性

| 属性 | 说明 |
|------|------|
| Health / MaxHealth | 生命值 |
| Mana / MaxMana | 法力值 |
| AttackPower | 攻击力 |
| Defense | 防御力 |
| AttackSpeed | 攻击速度 |
| MoveSpeed | 移动速度 |
| AttackRange | 攻击范围 |
| IncomingDamage | Meta属性，伤害计算用 |

### GameplayTags 分类

- State.*：状态标签（Dead, Stunned, Silenced, Rooted, Invulnerable）
- Ability.*：技能标签
- Cooldown.*：冷却标签
- Effect.*：效果标签（Damage, Heal, Buff, Debuff, Slow）
- Input.*：输入标签（用于绑定技能）
- Event.*：事件标签

### GameplayAbility

| 技能 | 说明 |
|------|------|
| GA_HeroAttack | 英雄普通攻击 |
| GA_HeroSkill_Q | Q技能（范围伤害） |
| GA_HeroSkill_W | W技能（增益效果） |
| GA_HeroSkill_E | E技能（位移/控制） |
| GA_TowerAttack | 塔自动攻击 |

### GameplayEffect

| 效果 | 类型 | 说明 |
|------|------|------|
| GE_Damage | Instant | 即时伤害 |
| GE_Heal | Instant | 即时治疗 |
| GE_Slow | Duration | 持续减速 |

---

## Enhanced Input 设计

### Input Actions

| Action | 类型 | 绑定 | GameplayTag |
|--------|------|------|-------------|
| IA_Move | Bool | 右键 | Input.Move |
| IA_Attack | Bool | 右键/A键 | Input.Attack |
| IA_Skill_Q | Bool | Q键 | Input.Skill.Q |
| IA_Skill_W | Bool | W键 | Input.Skill.W |
| IA_Skill_E | Bool | E键 | Input.Skill.E |
| IA_Skill_R | Bool | R键 | Input.Skill.R |
| IA_CameraPan | Vector2D | 屏幕边缘 | Input.Camera.Pan |
| IA_CameraZoom | Float | 滚轮 | Input.Camera.Zoom |
| IA_CameraFocus | Bool | 空格 | Input.Camera.Focus |
| IA_BuildMode | Bool | B键 | Input.Build |
| IA_Cancel | Bool | ESC | Input.Cancel |
| IA_Select | Bool | 左键 | Input.Select |

### 输入绑定流程

InputAction → GameplayTag → ValorisInputComponent → ASC → GameplayAbility

---

## 开发计划（90小时）

### 第一阶段：核心框架（第1-3周，20小时）

- 项目结构搭建，模块依赖
- GameMode / PlayerController / GameState
- Enhanced Input 系统
- RTS相机系统
- 角色基类 + ASC初始化
- 英雄点击移动
- DataAsset 基础类
- 测试场景

**里程碑**: 英雄可移动，相机可操控

### 第二阶段：GAS战斗系统（第4-6周，25小时）

- AttributeSet
- AbilitySystemComponent
- GameplayTags
- GameplayAbility 基类
- GA_HeroAttack + GE_Damage
- 敌人基类 + ASC + AI
- 敌人路径移动
- 波次管理器
- 基础UI

**里程碑**: 英雄能攻击敌人，敌人沿路径移动

### 第三阶段：塔防系统（第7-9周，25小时）

- 网格系统
- 建造预览
- 塔基类 + ASC
- GA_TowerAttack + 投射物
- 3种塔类型
- GE_Slow
- 塔升级系统
- 建造UI

**里程碑**: 完整塔防循环可玩

### 第四阶段：英雄技能（第10-11周，15小时）

- GA_HeroSkill_Q/W/E
- 技能UI
- 游戏流程（胜利/失败）
- 音效 + 特效

**里程碑**: 英雄有技能，游戏有完整流程

### 第五阶段：打磨发布（第12周，5小时）

- Bug修复
- 平衡调整
- 打包测试
- 发布

---

## MVP功能清单

### P0（必须实现）

- [ ] RTS风格相机
- [ ] 英雄点击移动
- [ ] 英雄自动攻击
- [ ] 敌人沿路径移动
- [ ] 敌人波次生成
- [ ] 防御塔建造
- [ ] 防御塔攻击
- [ ] 资源系统
- [ ] 基础UI
- [ ] 胜利/失败判定
- [ ] 一个可玩关卡

### P1（应该实现）

- [ ] 3种塔类型
- [ ] 塔升级
- [ ] 3种敌人类型
- [ ] 英雄3个技能
- [ ] 伤害飘字
- [ ] 音效

### P2（可延后）

- [ ] 多关卡
- [ ] 更多塔/敌人/技能
- [ ] 存档系统
- [ ] 成就/排行榜

---

## 命名规范

| 类型 | 前缀 | 示例 |
|------|------|------|
| Actor | A | ATowerBase |
| Component | U | UHealthComponent |
| Widget | U | UHeroStatusWidget |
| DataAsset | U | UHeroDataAsset |
| Interface | I | IDamageable |
| Enum | E | ETowerType |
| Struct | F | FHeroStats |
| GameplayAbility | U + GA_ | UGA_HeroAttack |
| GameplayEffect | U + GE_ | UGE_Damage |

---

## Content 目录规划

```
Content/
├── Blueprints/Heroes/      # 英雄蓝图
├── Blueprints/Towers/      # 塔蓝图
├── Blueprints/Enemies/     # 敌人蓝图
├── Data/                   # DataAsset
├── Input/                  # InputAction, IMC
├── UI/                     # Widget蓝图
├── Maps/                   # 关卡
├── Meshes/                 # 模型
├── Materials/              # 材质
├── Animations/             # 动画
└── Audio/                  # 音效
```

---

## 风险与应对

| 风险 | 应对 |
|------|------|
| 时间不足 | 优先P0，P1/P2延后 |
| 美术资源缺乏 | 使用商城免费资源或简单几何体 |
| GAS复杂度 | 简化技能效果 |
| 性能问题 | 控制同屏敌人数量 |

---

## 美术资源方案

- **英雄**: UE Mannequin 或商城免费角色
- **塔**: 简单几何体 + 材质区分
- **敌人**: 商城免费怪物
- **特效**: Niagara 模板
- **UI**: 简洁风格

---

## 开发日志

| 日期 | 时长 | 完成内容 |
|------|------|----------|
| 2025-11-26 | 40分钟 | 创建开发文档、GameMode、PlayerController，配置 GAS 模块依赖 |
| 2025-11-27 | 20分钟 | 添加 RTS 风格相机系统（SpectatorPawn、边缘平移、缩放、旋转） |

**累计开发时间**: 1小时 / 90小时
