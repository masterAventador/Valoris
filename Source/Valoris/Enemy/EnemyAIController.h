// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 敌人 AI 控制器
 * 负责敌人沿路径移动到基地
 */
UCLASS()
class VALORIS_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	// 开始沿路径移动
	UFUNCTION(BlueprintCallable, Category = "AI")
	void StartPathMovement();

	// 移动到下一个路径点
	void MoveToNextPathPoint();

protected:
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

private:
	// 当前路径点索引
	int32 CurrentPathIndex = 0;

	// 路径点数组（后续从关卡或 Spline 获取）
	UPROPERTY()
	TArray<FVector> PathPoints;
};
