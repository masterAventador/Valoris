// Copyright Valoris. All Rights Reserved.

#include "GA_Buff.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "../../ValorisGameplayTags.h"
#include "../../Effects/GE_Buff.h"

UGA_Buff::UGA_Buff()
{
	// 默认使用 GE_Buff
	BuffEffect = UGE_Buff::StaticClass();
}

void UGA_Buff::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	bBuffApplied = false;

	// 如果没有 Montage，直接应用效果
	if (!AbilityMontage)
	{
		if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		ApplyBuffEffect();
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 有 Montage，调用父类处理（等待动画事件触发效果）
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_Buff::OnEventReceived(FGameplayEventData Payload)
{
	// 收到任何事件时应用增益（通常由动画通知触发）
	if (!bBuffApplied)
	{
		ApplyBuffEffect();
		bBuffApplied = true;
	}
}

void UGA_Buff::ApplyBuffEffect()
{
	if (!BuffEffect)
	{
		return;
	}

	// 确定目标 ASC
	UAbilitySystemComponent* TargetASC = nullptr;

	if (bApplyToSelf)
	{
		TargetASC = GetAbilitySystemComponentFromActorInfo();
	}
	else
	{
		// 对目标施加
		const AActor* Target = GetCachedTarget();
		if (Target)
		{
			TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(Target));
		}
	}

	if (!TargetASC)
	{
		return;
	}

	// 创建增益效果
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(BuffEffect, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		// 设置持续时间
		if (BuffDuration > 0.0f)
		{
			SpecHandle.Data->SetDuration(BuffDuration, true);
		}

		// 应用效果
		TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
