// Copyright Valoris. All Rights Reserved.

#include "HeroAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UHeroAnimInstance::UHeroAnimInstance()
	: Speed(0.0f)
	, Direction(0.0f)
	, bIsMoving(false)
	, bIsInAir(false)
	, bIsDead(false)
{
}

void UHeroAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwningCharacter)
	{
		MovementComponent = OwningCharacter->GetCharacterMovement();
	}
}

void UHeroAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter || !MovementComponent)
	{
		return;
	}

	// 获取速度
	FVector Velocity = OwningCharacter->GetVelocity();
	Speed = Velocity.Size2D();

	// 计算移动方向（相对于角色朝向）
	if (Speed > 0.0f)
	{
		FRotator ActorRotation = OwningCharacter->GetActorRotation();
		Direction = UKismetMathLibrary::NormalizedDeltaRotator(
			Velocity.ToOrientationRotator(),
			ActorRotation
		).Yaw;
	}
	else
	{
		Direction = 0.0f;
	}

	// 是否在移动
	bIsMoving = Speed > 3.0f;

	// 是否在空中
	bIsInAir = MovementComponent->IsFalling();
}
