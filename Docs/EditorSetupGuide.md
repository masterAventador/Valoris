# Valoris 编辑器配置指南

本文档记录所有在 UE 编辑器中创建的蓝图、资产及其配置。

---

## 目录结构

```
Content/
├── Input/                      # 输入配置
│   ├── Actions/                # InputAction 资产
│   │   ├── IA_RightClick
│   │   ├── IA_CameraZoom
│   │   ├── IA_CameraRotate
│   │   └── IA_FocusHero
│   └── IMC_Valoris             # InputMappingContext
│
├── Blueprints/
│   ├── Core/                   # 核心蓝图
│   │   ├── BP_ValorisGameMode
│   │   └── BP_ValorisPlayerController
│   ├── Camera/
│   │   └── BP_ValorisSpectatorPawn
│   ├── Heroes/                 # 英雄蓝图（待创建）
│   ├── Towers/                 # 塔蓝图（待创建）
│   └── Enemies/                # 敌人蓝图（待创建）
│
└── Maps/
    └── TestMap                 # 测试地图
```

---

## 1. Input 配置

### 1.1 创建 InputAction

路径：`Content/Input/Actions/`

| 资产名称 | Value Type | 说明 |
|----------|------------|------|
| IA_RightClick | Bool (Digital) | 右键点击，用于移动/攻击 |
| IA_CameraZoom | Axis1D (Float) | 相机缩放，滚轮控制 |
| IA_CameraRotate | Axis1D (Float) | 相机旋转，中键拖拽 |
| IA_FocusHero | Bool (Digital) | 聚焦英雄，空格键 |

**创建方法**：
1. Content Browser 右键 → Input → Input Action
2. 命名并设置 Value Type

### 1.2 创建 InputMappingContext

路径：`Content/Input/IMC_Valoris`

**创建方法**：
1. Content Browser 右键 → Input → Input Mapping Context
2. 命名为 `IMC_Valoris`

**按键绑定配置**：

| InputAction | 按键 | Trigger | Modifier |
|-------------|------|---------|----------|
| IA_RightClick | Right Mouse Button | Pressed | - |
| IA_CameraZoom | Mouse Wheel Axis | - | - |
| IA_CameraRotate | Mouse X (当中键按下时) | - | 需要配合中键按下 |
| IA_FocusHero | Space Bar | Pressed | - |

---

## 2. 核心蓝图

### 2.1 BP_ValorisPlayerController

路径：`Content/Blueprints/Core/BP_ValorisPlayerController`

**创建方法**：
1. Content Browser 右键 → Blueprint Class
2. 搜索并选择父类 `ValorisPlayerController`
3. 命名为 `BP_ValorisPlayerController`

**配置**：
- Default Mapping Context → `IMC_Valoris`
- Right Click Action → `IA_RightClick`
- Camera Zoom Action → `IA_CameraZoom`
- Camera Rotate Action → `IA_CameraRotate`
- Focus Hero Action → `IA_FocusHero`

### 2.2 BP_ValorisSpectatorPawn

路径：`Content/Blueprints/Camera/BP_ValorisSpectatorPawn`

**创建方法**：
1. Content Browser 右键 → Blueprint Class
2. 搜索并选择父类 `ValorisSpectatorPawn`
3. 命名为 `BP_ValorisSpectatorPawn`

**配置**（可选调整）：
- Pan Speed: 1500
- Zoom Speed: 200
- Min Zoom Distance: 500
- Max Zoom Distance: 3000
- Rotate Speed: 100
- Camera Pitch: -50
- Edge Pan Threshold: 20
- Enable Edge Panning: true

### 2.3 BP_ValorisGameMode

路径：`Content/Blueprints/Core/BP_ValorisGameMode`

**创建方法**：
1. Content Browser 右键 → Blueprint Class
2. 搜索并选择父类 `ValorisGameMode`
3. 命名为 `BP_ValorisGameMode`

**配置**：
- Default Pawn Class → `BP_ValorisSpectatorPawn`
- Player Controller Class → `BP_ValorisPlayerController`

---

## 3. 地图配置

### 3.1 TestMap 设置

1. 打开 `Content/Maps/TestMap`
2. Window → World Settings
3. GameMode Override → `BP_ValorisGameMode`

### 3.2 项目默认地图

1. Edit → Project Settings → Maps & Modes
2. Default GameMode → `BP_ValorisGameMode`
3. Editor Startup Map → `TestMap`
4. Game Default Map → `TestMap`

---

## 4. 配置状态检查清单

### Input 配置
- [ ] 创建 `Content/Input/Actions/` 目录
- [ ] 创建 `IA_RightClick` (Bool)
- [ ] 创建 `IA_CameraZoom` (Axis1D)
- [ ] 创建 `IA_CameraRotate` (Axis1D)
- [ ] 创建 `IA_FocusHero` (Bool)
- [ ] 创建 `IMC_Valoris` 并配置按键绑定

### 蓝图配置
- [ ] 创建 `Content/Blueprints/Core/` 目录
- [ ] 创建 `Content/Blueprints/Camera/` 目录
- [ ] 创建 `BP_ValorisPlayerController` 并配置 InputAction 引用
- [ ] 创建 `BP_ValorisSpectatorPawn`
- [ ] 创建 `BP_ValorisGameMode` 并配置 Pawn 和 Controller

### 地图配置
- [ ] TestMap 设置 GameMode Override
- [ ] 项目设置默认 GameMode

---

## 5. 测试验证

配置完成后，运行游戏测试：

1. **边缘平移**：鼠标移到屏幕边缘，相机应该自动移动
2. **滚轮缩放**：滚动鼠标滚轮，相机应该平滑缩放
3. **中键旋转**：按住中键左右拖动，相机应该旋转
4. **空格聚焦**：按空格键，相机应该回到英雄位置（需要先有英雄）
5. **右键点击**：右键点击地面，Output Log 应该显示点击坐标

---

## 6. 英雄配置

### 6.1 BP_AricHero

路径：`Content/Blueprints/Heroes/BP_AricHero`

**创建方法**：
1. Content Browser 右键 → Blueprint Class
2. 搜索并选择父类 `AricHero`
3. 命名为 `BP_AricHero`

**配置**：
- Mesh: 设置角色模型
- Default Abilities: 添加 `GA_HeroAttack`（或其蓝图子类）
- Default Attribute Effect: 设置初始属性的 GameplayEffect

### 6.2 GA_HeroAttack 配置

如果使用 C++ 类 `GA_HeroAttack` 的蓝图子类：

1. Content Browser 右键 → Blueprint Class
2. 搜索并选择父类 `GA_HeroAttack`
3. 命名为 `BP_GA_HeroAttack`

**配置**：
- Damage Effect: 设置为 `GE_Damage` 类

---

## 7. 敌人路径配置

### 7.1 创建 EnemyPath（Spline 路径）

**步骤**：

1. **放置 EnemyPath Actor**
   - Content Browser 右键 → Blueprint Class → 选择父类 `EnemyPath`
   - 或者直接在场景中：Place Actors 面板搜索 `EnemyPath`，拖入场景
   - 也可以直接使用 C++ 类：在 Place Actors 面板搜索 `EnemyPath`

2. **编辑 Spline 路径点**
   - 选中场景中的 EnemyPath Actor
   - 在 Details 面板找到 Spline Component
   - 或者直接在视口中操作 Spline

3. **Spline 编辑操作**：

   | 操作 | 说明 |
   |------|------|
   | 选中 Spline 点 | 点击 Spline 上的白色方块 |
   | 移动点 | 选中后拖动移动手柄 |
   | 添加点 | 在 Spline 线上 **Alt + 左键点击** |
   | 删除点 | 选中点后按 **Delete** |
   | 调整曲线 | 拖动点的切线手柄（黄色线） |

4. **Spline 点类型**（选中点后在 Details 中设置）：

   | 类型 | 效果 |
   |------|------|
   | Curve | 平滑曲线（默认） |
   | Linear | 直线连接 |
   | Constant | 阶梯状 |

5. **调试显示**：
   - EnemyPath 默认开启 `SetDrawDebug(true)`
   - 运行时会显示绿色的 Spline 线

### 7.2 创建敌人蓝图

路径：`Content/Blueprints/Enemies/BP_EnemyBase`

**创建方法**：
1. Content Browser 右键 → Blueprint Class
2. 搜索并选择父类 `EnemyBase`
3. 命名为 `BP_EnemyBase`

**配置**：
- Mesh: 设置敌人模型
- Base Damage: 到达基地造成的伤害（默认 1）
- Gold Reward: 击杀奖励金币（默认 10）
- Default Attribute Effect: 设置初始属性（包含 MoveSpeed）

### 7.3 让敌人沿路径移动

**方法一：场景中手动设置**

1. 在场景中放置 `BP_EnemyBase`
2. 在场景中放置 `EnemyPath` 并编辑路径
3. 选中敌人，在 Details 面板找到 `Current Path`
4. 使用吸管工具选择场景中的 EnemyPath

**方法二：蓝图中设置（推荐用于波次生成）**

在敌人蓝图的 Event Graph 中：

```
Event BeginPlay
    ↓
Get All Actors Of Class (EnemyPath)
    ↓
Get (取第一个，或根据名称/Tag筛选)
    ↓
Set Path (传入 EnemyPath 引用)
```

**方法三：通过生成器设置（后续波次系统使用）**

```cpp
// C++ 代码示例
AEnemyBase* Enemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, SpawnLocation, SpawnRotation);
if (Enemy && EnemyPath)
{
    Enemy->SetPath(EnemyPath);
}
```

### 7.4 测试敌人路径移动

1. 在 TestMap 中放置一个 `EnemyPath`
2. 编辑 Spline，设置 3-4 个路径点
3. 放置一个 `BP_EnemyBase`
4. 设置敌人的 `Current Path` 指向 EnemyPath
5. 运行游戏
6. 敌人应该从 Spline 起点开始，沿路径移动到终点
7. 到达终点后敌人会自动销毁

### 7.5 路径设计建议

- **起点**：通常在地图边缘，敌人生成点
- **终点**：玩家基地位置
- **路径**：可以设计多条路径，不同波次走不同路线
- **拐弯**：使用 Curve 类型的点让转弯更自然
- **长度**：路径不要太短，给玩家足够的反应时间

---

## 8. 完整测试流程

### 8.1 英雄攻击敌人测试

1. 确保场景中有：
   - NavMeshBoundsVolume（覆盖整个可行走区域）
   - BP_AricHero（英雄）
   - BP_EnemyBase（敌人，可以不设置路径，让它原地不动）

2. 运行游戏

3. 右键点击敌人，英雄应该：
   - 移动到敌人附近
   - 自动开始攻击
   - 每次攻击后有冷却间隔

### 8.2 敌人沿路径移动测试

1. 确保场景中有：
   - EnemyPath（已编辑好路径）
   - BP_EnemyBase（Current Path 指向 EnemyPath）

2. 运行游戏

3. 敌人应该：
   - 从路径起点开始移动
   - 沿 Spline 曲线移动
   - 到达终点后消失

---

## 9. 防御塔建造配置

### 9.1 创建 InputAction

路径：`Content/Input/Actions/`

| 资产名称 | Value Type | 按键绑定 | 说明 |
|----------|------------|----------|------|
| IA_LeftClick | Bool (Digital) | Left Mouse Button | 左键确认建造 |
| IA_Cancel | Bool (Digital) | Escape | ESC 取消建造 |

在 `IMC_Valoris` 中添加这两个 InputAction 的绑定。

### 9.2 创建塔蓝图

路径：`Content/Blueprints/Towers/BP_TowerBase`

**创建方法**：
1. Content Browser 右键 → Blueprint Class
2. 搜索并选择父类 `TowerBase`
3. 命名为 `BP_TowerBase`

**配置**：
- Tower Mesh: 设置一个简单的 Static Mesh（如 Cylinder 或 Cube）
- Build Cost: 建造花费（默认 100）
- Default Abilities: 后续添加 `GA_TowerAttack`
- Default Attribute Effect: 设置塔的初始属性

### 9.3 创建建造预览蓝图

路径：`Content/Blueprints/Building/BP_BuildPreview`

**创建方法**：
1. Content Browser 右键 → Blueprint Class
2. 搜索并选择父类 `BuildPreview`
3. 命名为 `BP_BuildPreview`

**配置**：
- Preview Mesh: 默认使用圆柱体，可自定义
- Valid Material: 创建一个绿色半透明材质（表示可建造）
- Invalid Material: 创建一个红色半透明材质（表示不可建造）
- Overlap Check Radius: 检测重叠的半径（默认 100）

**创建预览材质**：

1. Content Browser 右键 → Material
2. 命名为 `M_BuildPreview_Valid`
3. 打开材质编辑器：
   - Blend Mode: Translucent
   - Base Color: 绿色 (0, 1, 0)
   - Opacity: 0.5
4. 复制并命名为 `M_BuildPreview_Invalid`
   - Base Color: 红色 (1, 0, 0)

### 9.4 配置 PlayerController

在 `BP_ValorisPlayerController` 中配置：

| 属性 | 值 |
|------|------|
| Left Click Action | IA_LeftClick |
| Cancel Action | IA_Cancel |
| Build Preview Class | BP_BuildPreview |

### 9.5 测试建造系统

**方法一：关卡蓝图测试**

1. 打开关卡蓝图（Blueprints → Open Level Blueprint）
2. 添加以下逻辑：

```
Event BeginPlay
    ↓
Delay (2秒，给点准备时间)
    ↓
Get Player Controller → Cast To ValorisPlayerController
    ↓
Enter Build Mode (Tower Class = BP_TowerBase)
```

**方法二：按键触发**

1. 创建一个新的 InputAction `IA_BuildMode`（绑定 B 键）
2. 在 PlayerController 中处理该输入，调用 `EnterBuildMode`

**测试流程**：

1. 运行游戏
2. 进入建造模式后，鼠标位置会显示预览圆柱
3. 预览跟随鼠标移动
4. 绿色 = 可建造，红色 = 不可建造（与其他塔重叠）
5. 左键点击确认建造
6. ESC 取消建造模式

---

## 更新记录

| 日期 | 更新内容 |
|------|----------|
| 2025-11-28 | 创建文档，添加 Input 和核心蓝图配置说明 |
| 2025-12-02 | 添加英雄配置、敌人路径配置、Spline 编辑说明 |
| 2025-12-07 | 添加防御塔建造配置、建造预览材质创建说明 |
