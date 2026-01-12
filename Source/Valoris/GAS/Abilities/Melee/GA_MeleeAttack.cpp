// Copyright Valoris. All Rights Reserved.

#include "GA_MeleeAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "../../ValorisGameplayTags.h"
#include "../../ValorisAttributeSet.h"
#include "../../GE_Damage.h"
#include "../../../Character/ValorisCharacterBase.h"

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

	// 获取目标（优先使用事件中的目标，其次使用缓存的目标）
	AActor* Target = const_cast<AActor*>(Payload.Target.Get());
	if (!Target)
	{
		Target = const_cast<AActor*>(GetCachedTarget());
	}

	if (Target)
	{
		ApplyDamageToTarget(Target);
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
