// Copyright Valoris. All Rights Reserved.

#include "GA_Charge.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "../../ValorisGameplayTags.h"
#include "../../ValorisAttributeSet.h"
#include "../../GE_Damage.h"
#include "../../../Character/ValorisCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UGA_Charge::UGA_Charge()
{
	// 默认使用 GE_Damage
	ImpactDamageEffect = UGE_Damage::StaticClass();
}

void UGA_Charge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
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

	// 确定冲锋方向
	// 优先使用触发事件中的目标位置
	if (TriggerEventData && TriggerEventData->Target.Get() != nullptr)
	{
		ChargeDirection = (TriggerEventData->Target->GetActorLocation() - AvatarActor->GetActorLocation()).GetSafeNormal();
	}
	else
	{
		// 否则使用角色朝向
		ChargeDirection = AvatarActor->GetActorForwardVector();
	}

	ChargeStartLocation = AvatarActor->GetActorLocation();
	ChargedDistance = 0.0f;
	HitTargets.Empty();

	// 计算冲锋持续时间
	float Duration = ChargeDuration;
	if (Duration <= 0.0f)
	{
		Duration = ChargeDistance / ChargeSpeed;
	}

	// 播放动画（如果有）
	if (AbilityMontage)
	{
		// 调用父类处理 Montage
		Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	}

	// 启动冲锋定时器
	if (UWorld* World = GetWorld())
	{
		// 使用高频率定时器进行平滑移动
		World->GetTimerManager().SetTimer(
			ChargeTimerHandle,
			this,
			&UGA_Charge::PerformCharge,
			0.016f, // ~60fps
			true
		);

		// 设置结束定时器
		FTimerHandle EndTimerHandle;
		World->GetTimerManager().SetTimer(
			EndTimerHandle,
			this,
			&UGA_Charge::OnChargeEnd,
			Duration,
			false
		);
	}
}

void UGA_Charge::OnEventReceived(FGameplayEventData Payload)
{
	// 冲锋技能不依赖 AnimNotify 事件触发伤害
	// 伤害在 CheckImpact 中实时检测
}

void UGA_Charge::PerformCharge()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		OnChargeEnd();
		return;
	}

	// 计算本帧移动距离
	float DeltaTime = 0.016f;
	float MoveDistance = ChargeSpeed * DeltaTime;

	// 检查是否超过最大距离
	if (ChargedDistance + MoveDistance > ChargeDistance)
	{
		MoveDistance = ChargeDistance - ChargedDistance;
	}

	// 移动角色
	FVector NewLocation = AvatarActor->GetActorLocation() + ChargeDirection * MoveDistance;
	AvatarActor->SetActorLocation(NewLocation, true);

	ChargedDistance += MoveDistance;

	// 旋转角色朝向冲锋方向
	FRotator NewRotation = ChargeDirection.Rotation();
	AvatarActor->SetActorRotation(NewRotation);

	// 检测撞击
	CheckImpact();

	// 检查是否完成
	if (ChargedDistance >= ChargeDistance)
	{
		OnChargeEnd();
	}
}

void UGA_Charge::CheckImpact()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		return;
	}

	// 球形检测
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);

	FVector Origin = AvatarActor->GetActorLocation();

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Origin,
		Origin,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(ImpactRadius),
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor)
		{
			continue;
		}

		// 检查是否已经撞击过
		bool bAlreadyHit = false;
		for (const TWeakObjectPtr<AActor>& HitTarget : HitTargets)
		{
			if (HitTarget.Get() == HitActor)
			{
				bAlreadyHit = true;
				break;
			}
		}

		if (bAlreadyHit)
		{
			continue;
		}

		// 检查是否有 ASC（是否是有效目标）
		if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
		{
			ApplyImpactDamage(HitActor);
			HitTargets.Add(HitActor);
		}
	}
}

void UGA_Charge::ApplyImpactDamage(AActor* Target)
{
	if (!Target || !ImpactDamageEffect)
	{
		return;
	}

	// 获取目标的 ASC
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC)
	{
		return;
	}

	// 计算伤害值
	float BaseDamage = 0.0f;
	if (UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (const UValorisAttributeSet* Attributes = SourceASC->GetSet<UValorisAttributeSet>())
		{
			BaseDamage = Attributes->GetAttackPower() * DamageMultiplier;
		}
	}

	// 应用伤害效果
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(ImpactDamageEffect, GetAbilityLevel());
	if (DamageSpecHandle.IsValid())
	{
		DamageSpecHandle.Data->SetSetByCallerMagnitude(FValorisGameplayTags::Data_Damage, BaseDamage);
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
	}
}

void UGA_Charge::OnChargeEnd()
{
	// 停止定时器
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChargeTimerHandle);
	}

	// 结束技能
	EndAbility(GetCurrentAbilitySpecHandle(), CurrentActorInfo, GetCurrentActivationInfo(), true, false);
}
