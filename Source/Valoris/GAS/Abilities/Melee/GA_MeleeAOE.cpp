// Copyright Valoris. All Rights Reserved.

#include "GA_MeleeAOE.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "../../ValorisGameplayTags.h"
#include "../../ValorisAttributeSet.h"
#include "../../GE_Damage.h"
#include "../../../Character/ValorisCharacterBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Algo/Sort.h"

UGA_MeleeAOE::UGA_MeleeAOE()
{
	// 默认使用 GE_Damage
	DamageEffect = UGE_Damage::StaticClass();
}

void UGA_MeleeAOE::OnEventReceived(FGameplayEventData Payload)
{
	// 只处理攻击命中事件
	if (!Payload.EventTag.MatchesTag(FValorisGameplayTags::Event_Attack_Hit))
	{
		return;
	}

	ApplyAOEDamage();
}

void UGA_MeleeAOE::ApplyAOEDamage()
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
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

	// 球形检测范围内的目标
	TArray<FHitResult> HitResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(AvatarActor);

	FVector Origin = AvatarActor->GetActorLocation();

	bool bHit = GetWorld()->SweepMultiByChannel(
		HitResults,
		Origin,
		Origin,
		FQuat::Identity,
		TargetChannel,
		FCollisionShape::MakeSphere(AOERadius),
		QueryParams
	);

	if (!bHit)
	{
		return;
	}

	// 收集目标（去重）
	TArray<AActor*> Targets;
	for (const FHitResult& Hit : HitResults)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && !Targets.Contains(HitActor))
		{
			// 检查目标是否是敌方（这里简单判断是否有 ASC）
			if (UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor))
			{
				Targets.Add(HitActor);
			}
		}
	}

	// 应用最大目标数量限制
	if (MaxTargets > 0 && Targets.Num() > MaxTargets)
	{
		// 按距离排序，优先攻击最近的目标
		FVector SortOrigin = Origin;
		Algo::Sort(Targets, [SortOrigin](AActor* A, AActor* B)
		{
			float DistA = FVector::DistSquared(A->GetActorLocation(), SortOrigin);
			float DistB = FVector::DistSquared(B->GetActorLocation(), SortOrigin);
			return DistA < DistB;
		});

		Targets.SetNum(MaxTargets);
	}

	// 对每个目标应用伤害
	for (AActor* Target : Targets)
	{
		ApplyDamageToTarget(Target, BaseDamage);
	}
}

void UGA_MeleeAOE::ApplyDamageToTarget(AActor* Target, float Damage)
{
	if (!Target || !DamageEffect)
	{
		return;
	}

	// 获取目标的 ASC
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!TargetASC)
	{
		return;
	}

	// 应用伤害效果
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
	if (DamageSpecHandle.IsValid())
	{
		DamageSpecHandle.Data->SetSetByCallerMagnitude(FValorisGameplayTags::Data_Damage, Damage);
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
	}
}
