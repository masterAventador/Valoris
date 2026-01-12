# Aric 技能蓝图配置指南

## 目录结构

在 Content Browser 中创建以下目录：
```
Content/Blueprints/GAS/Abilities/Heroes/Aric/
```

## 技能蓝图列表

| 蓝图名 | 父类 | 技能 | 类型 |
|-------|------|------|------|
| BP_GA_Aric_Attack | GA_MeleeAttack | 普攻 | 近战单体 |
| BP_GA_Aric_ShieldBash | GA_MeleeAttack | Q - 盾击 | 近战单体+眩晕 |
| BP_GA_Aric_WhirlwindSlash | GA_MeleeAOE | W - 旋风斩 | 近战AOE |
| BP_GA_Aric_Charge | GA_Charge | E - 冲锋 | 位移+伤害 |
| BP_GA_Aric_BattleCry | GA_Buff | R - 战吼 | 自身增益 |

---

## 创建步骤

### 通用步骤（每个技能都要做）

1. **创建蓝图**
   - 右键 → Blueprint Class
   - 搜索并选择对应的父类（如 `GA_MeleeAttack`）
   - 命名为对应的蓝图名

2. **设置 AbilityTags**（重要！用于冷却识别）
   - 打开蓝图 → Class Defaults
   - 找到 `Ability Tags`（在 Tags 分类下）
   - 添加对应的技能 Tag

3. **设置动画（可选）**
   - 找到 `Ability Montage`
   - 选择对应的攻击动画 Montage

---

## 各技能详细配置

### 1. BP_GA_Aric_Attack（普攻）

**父类：** `GA_MeleeAttack`

**Class Defaults 配置：**
| 属性 | 值 | 说明 |
|-----|-----|-----|
| Damage Multiplier | 1.0 | 伤害倍率 |
| Attack Range | 200 | 攻击范围 |
| Cooldown Duration | 0 | 无冷却 |
| Damage Effect | GE_Damage | 伤害效果 |
| Control Effect | None | 无控制效果 |

**Ability Tags：**
- `Ability.Melee.Single.Aric.Attack`

---

### 2. BP_GA_Aric_ShieldBash（Q - 盾击）

**父类：** `GA_MeleeAttack`

**Class Defaults 配置：**
| 属性 | 值 | 说明 |
|-----|-----|-----|
| Damage Multiplier | 1.5 | 伤害倍率 |
| Attack Range | 250 | 攻击范围 |
| Cooldown Duration | 8.0 | 8秒冷却 |
| Damage Effect | GE_Damage | 伤害效果 |
| Control Effect | *需创建眩晕GE* | 眩晕效果 |

**Ability Tags：**
- `Ability.Melee.Single.Aric.ShieldBash`

**需要额外创建：** 眩晕 GameplayEffect（GE_Stun）

---

### 3. BP_GA_Aric_WhirlwindSlash（W - 旋风斩）

**父类：** `GA_MeleeAOE`

**Class Defaults 配置：**
| 属性 | 值 | 说明 |
|-----|-----|-----|
| Damage Multiplier | 0.8 | 伤害倍率（AOE稍低） |
| AOE Radius | 350 | AOE半径 |
| Max Targets | 0 | 无限制 |
| Cooldown Duration | 10.0 | 10秒冷却 |
| Damage Effect | GE_Damage | 伤害效果 |
| Target Channel | ECC_Pawn | 碰撞通道 |

**Ability Tags：**
- `Ability.Melee.AOE.Aric.WhirlwindSlash`

---

### 4. BP_GA_Aric_Charge（E - 冲锋）

**父类：** `GA_Charge`

**Class Defaults 配置：**
| 属性 | 值 | 说明 |
|-----|-----|-----|
| Charge Distance | 800 | 冲锋距离 |
| Charge Speed | 2000 | 冲锋速度 |
| Charge Duration | 0.4 | 冲锋时长 |
| Damage Multiplier | 1.2 | 伤害倍率 |
| Impact Radius | 150 | 冲撞伤害半径 |
| Cooldown Duration | 12.0 | 12秒冷却 |
| Impact Damage Effect | GE_Damage | 冲撞伤害效果 |
| Target Channel | ECC_Pawn | 碰撞通道 |

**Ability Tags：**
- `Ability.Movement.Aric.Charge`

---

### 5. BP_GA_Aric_BattleCry（R - 战吼）

**父类：** `GA_Buff`

**Class Defaults 配置：**
| 属性 | 值 | 说明 |
|-----|-----|-----|
| Buff Duration | 8.0 | 增益持续8秒 |
| bApply To Self | true | 应用给自己 |
| Cooldown Duration | 30.0 | 30秒冷却 |
| Buff Effect | *需创建增益GE* | 增益效果 |

**Ability Tags：**
- `Ability.Buff.Aric.BattleCry`

**需要额外创建：** 增益 GameplayEffect（如 GE_BattleCry_Buff）
- 可以增加攻击力、攻速等属性
- 继承自 GE_Buff

---

## 配置 BP_Aric 角色

完成技能蓝图创建后，需要在 Aric 角色蓝图中配置：

1. 打开 `BP_Aric`
2. 找到 `Default Abilities` 数组
3. 添加以下技能：
   - BP_GA_Aric_Attack（普攻）
   - BP_GA_Aric_ShieldBash（Q）
   - BP_GA_Aric_WhirlwindSlash（W）
   - BP_GA_Aric_Charge（E）
   - BP_GA_Aric_BattleCry（R）

---

## 输入绑定

在 PlayerController 或 Character 中绑定输入到技能激活：

| 输入 | 技能 | 激活方式 |
|-----|------|---------|
| 左键 | 普攻 | TryActivateAbilityByClass |
| Q | 盾击 | TryActivateAbilityByClass |
| W | 旋风斩 | TryActivateAbilityByClass |
| E | 冲锋 | TryActivateAbilityByClass |
| R | 战吼 | TryActivateAbilityByClass |

---

## 测试清单

- [ ] 普攻能正常造成伤害
- [ ] Q 技能造成伤害并眩晕目标
- [ ] W 技能对周围多个目标造成 AOE 伤害
- [ ] E 技能向前冲锋并对路径上的敌人造成伤害
- [ ] R 技能给自己添加增益效果
- [ ] 所有技能冷却正常工作
- [ ] 冷却期间无法再次释放技能
