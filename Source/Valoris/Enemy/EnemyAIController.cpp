// Copyright Valoris. All Rights Reserved.

#include "EnemyAIController.h"
#include "EnemyBase.h"
#include "Navigation/PathFollowingComponent.h"

void AEnemyAIController::StartPathMovement()
{
	CurrentPathIndex = 0;

	if (PathPoints.Num() > 0)
	{
		MoveToNextPathPoint();
	}
}

void AEnemyAIController::MoveToNextPathPoint()
{
	if (CurrentPathIndex < PathPoints.Num())
	{
		MoveToLocation(PathPoints[CurrentPathIndex], 50.f, true, true, false, true);
	}
	else
	{
		// 到达终点（基地），触发对基地的伤害
		// TODO: 通知 GameMode 敌人到达基地
		if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Enemy %s reached the base!"), *Enemy->GetName());
		}
	}
}

void AEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	if (Result.IsSuccess())
	{
		CurrentPathIndex++;
		MoveToNextPathPoint();
	}
}
