// Copyright Valoris. All Rights Reserved.

#include "EnemyAnimInstance.h"
#include "GameFramework/Pawn.h"

UEnemyAnimInstance::UEnemyAnimInstance()
	: Speed(0.0f)
	, bIsMoving(false)
	, bIsDead(false)
{
}

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningPawn = TryGetPawnOwner();
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningPawn)
	{
		return;
	}

	// 获取速度
	FVector Velocity = OwningPawn->GetVelocity();
	Speed = Velocity.Size2D();

	// 是否在移动
	bIsMoving = Speed > 3.0f;
}
