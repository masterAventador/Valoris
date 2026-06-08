// Copyright Valoris. All Rights Reserved.

#include "GA_MeleeAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "../../ValorisGameplayTags.h"
#include "../../ValorisAttributeSet.h"
#include "../../GE_Damage.h"
#include "Engine/World.h"

UGA_MeleeAttack::UGA_MeleeAttack()
{
	// 默认使用 GE_Damage
	DamageEffect = UGE_Damage::StaticClass();
}

void UGA_MeleeAttack::OnEventReceived(FGameplayEventData Payload)
{
	// 只处理攻击命中事件
	if (!Payload.EventTag.MatchesTag(FValorisGameplayTags::Event_Attack_Hit))
	{
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return;
	}

	// 玩家挥砍没有预设目标：朝角色前方（= 鼠标朝向，见 AricHero::Tick）做球形扫描，
	// 对范围内所有带 ASC 的目标逐个造成伤害
	const FVector Origin = Avatar->GetActorLocation() + Avatar->GetActorForwardVector() * (AttackRange * 0.5f);
	const FCollisionShape Sphere = FCollisionShape::MakeSphere(AttackRange * 0.5f);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Avatar);

	TArray<FHitResult> Hits;
	Avatar->GetWorld()->SweepMultiByChannel(Hits, Origin, Origin, FQuat::Identity, ECC_Pawn, Sphere, Params);

	TSet<AActor*> Damaged;
	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (Target && !Damaged.Contains(Target)
			&& UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
		{
			ApplyDamageToTarget(Target);
			Damaged.Add(Target);
		}
	}
}

void UGA_MeleeAttack::ApplyDamageToTarget(AActor* Target)
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
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
	if (DamageSpecHandle.IsValid())
	{
		// 通过 SetByCaller 设置伤害值
		DamageSpecHandle.Data->SetSetByCallerMagnitude(FValorisGameplayTags::Data_Damage, BaseDamage);
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
	}

	// 应用控制效果（如果配置了）
	if (ControlEffect)
	{
		FGameplayEffectSpecHandle ControlSpecHandle = MakeOutgoingGameplayEffectSpec(ControlEffect, GetAbilityLevel());
		if (ControlSpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*ControlSpecHandle.Data.Get());
		}
	}
}
