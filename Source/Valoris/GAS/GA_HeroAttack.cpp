// Copyright Valoris. All Rights Reserved.

#include "GA_HeroAttack.h"
#include "AbilitySystemComponent.h"
#include "../Character/ValorisCharacterBase.h"
#include "ValorisAttributeSet.h"
#include "ValorisGameplayTags.h"
#include "GE_Cooldown_Attack.h"

UGA_HeroAttack::UGA_HeroAttack()
{
	// 设置事件触发
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FValorisGameplayTags::Get().Event_Attack;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// 设置冷却
	CooldownGameplayEffectClass = UGE_Cooldown_Attack::StaticClass();
	CooldownTags.AddTag(FValorisGameplayTags::Get().Cooldown_Attack);
}

void UGA_HeroAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 获取攻击目标（从 EventData 中传入）
	AActor* TargetActor = nullptr;
	if (TriggerEventData && TriggerEventData->Target)
	{
		TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
	}

	if (!TargetActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 获取目标的 ASC
	UAbilitySystemComponent* TargetASC = nullptr;
	if (AValorisCharacterBase* TargetCharacter = Cast<AValorisCharacterBase>(TargetActor))
	{
		TargetASC = TargetCharacter->GetAbilitySystemComponent();
	}

	if (!TargetASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 应用伤害效果（伤害值由 MMC_Damage 自动计算）
	if (DamageEffect)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

const FGameplayTagContainer* UGA_HeroAttack::GetCooldownTags() const
{
	return &CooldownTags;
}

void UGA_HeroAttack::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CooldownGameplayEffectClass)
	{
		return;
	}

	// 计算冷却时间（基于攻速）
	float CooldownDuration = 1.0f;
	if (AValorisCharacterBase* Character = Cast<AValorisCharacterBase>(ActorInfo->AvatarActor.Get()))
	{
		if (UValorisAttributeSet* Attributes = Character->GetAttributeSet())
		{
			float AttackSpeed = FMath::Max(0.1f, Attributes->GetAttackSpeed());
			CooldownDuration = 1.0f / AttackSpeed;
		}
	}

	// 创建冷却效果
	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(CooldownGameplayEffectClass, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data.Get()->SetDuration(CooldownDuration, true);
		ApplyGameplayEffectSpecToOwner(Handle, ActorInfo, ActivationInfo, SpecHandle);
	}
}
