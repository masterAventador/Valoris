// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 敌人 AI 控制器
 * 敌人沿 Spline 路径移动，移动逻辑在 EnemyBase::Tick 中实现
 * 此控制器预留给后续扩展（如敌人 AI 行为、战斗逻辑等）
 */
UCLASS()
class VALORIS_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();
};
