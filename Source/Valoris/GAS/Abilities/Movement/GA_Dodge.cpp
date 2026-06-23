// Copyright Valoris. All Rights Reserved.

#include "GA_Dodge.h"
#include "AbilitySystemComponent.h"
#include "../../ValorisGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UGA_Dodge::UGA_Dodge()
{
}

void UGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 方向取角色当前移动输入（WASD），零输入回退角色朝向
	FVector InputDir = FVector::ZeroVector;
	if (ACharacter* Char = Cast<ACharacter>(AvatarActor))
	{
		if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
		{
			InputDir = Move->GetLastInputVector();
		}
	}
	InputDir.Z = 0.f;
	if (InputDir.IsNearlyZero())
	{
		InputDir = AvatarActor->GetActorForwardVector();
	}
	DodgeDirection = InputDir.GetSafeNormal();
	DodgedDistance = 0.0f;

	// 挂无敌帧
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(FValorisGameplayTags::State_Invincible);
	}

	float Duration = DodgeDuration;
	if (Duration <= 0.0f)
	{
		Duration = DodgeDistance / DodgeSpeed;
	}

	// 播放动画（如有）
	if (AbilityMontage)
	{
		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(DodgeTimerHandle, this, &UGA_Dodge::PerformDodge, 0.016f, true);

		FTimerHandle EndTimerHandle;
		World->GetTimerManager().SetTimer(EndTimerHandle, this, &UGA_Dodge::OnDodgeEnd, Duration, false);
	}
}

void UGA_Dodge::OnEventReceived(FGameplayEventData Payload)
{
	// 闪避不依赖 AnimNotify 事件
}

void UGA_Dodge::PerformDodge()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		OnDodgeEnd();
		return;
	}

	const float DeltaTime = 0.016f;
	float MoveDistance = DodgeSpeed * DeltaTime;
	if (DodgedDistance + MoveDistance > DodgeDistance)
	{
		MoveDistance = DodgeDistance - DodgedDistance;
	}

	const FVector NewLocation = AvatarActor->GetActorLocation() + DodgeDirection * MoveDistance;
	AvatarActor->SetActorLocation(NewLocation, true);
	DodgedDistance += MoveDistance;

	if (DodgedDistance >= DodgeDistance)
	{
		OnDodgeEnd();
	}
}

void UGA_Dodge::OnDodgeEnd()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DodgeTimerHandle);
	}

	// 移除无敌帧
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(FValorisGameplayTags::State_Invincible);
	}

	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo, GetCurrentActivationInfo(), true, false);
}
