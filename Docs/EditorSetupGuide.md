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

## 更新记录

| 日期 | 更新内容 |
|------|----------|
| 2025-11-28 | 创建文档，添加 Input 和核心蓝图配置说明 |
