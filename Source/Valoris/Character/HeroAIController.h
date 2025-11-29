// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "HeroAIController.generated.h"

/**
 * 英雄 AI 控制器
 * 负责处理英雄的导航移动
 */
UCLASS()
class VALORIS_API AHeroAIController : public AAIController
{
	GENERATED_BODY()

public:
	// 移动到目标位置
	void MoveToTargetLocation(const FVector& TargetLocation);

	// 停止移动
	void StopMovement();
};
