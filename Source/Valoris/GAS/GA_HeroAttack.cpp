// Copyright Valoris. All Rights Reserved.

#include "GA_HeroAttack.h"
#include "AbilitySystemComponent.h"
#include "../Character/ValorisCharacterBase.h"
#include "ValorisAttributeSet.h"
#include "ValorisGameplayTags.h"
#include "GE_Cooldown_Attack.h"
#include "GE_Damage.h"

UGA_HeroAttack::UGA_HeroAttack()
{
	// 设置事件触发
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FValorisGameplayTags::Event_Attack;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);

	// 设置冷却
	CooldownGameplayEffectClass = UGE_Cooldown_Attack::StaticClass();
	CooldownTags.AddTag(FValorisGameplayTags::Cooldown_Attack);

	// 设置伤害效果
	DamageEffect = UGE_Damage::StaticClass();
}

void UGA_HeroAttack::OnEventReceived(FGameplayEventData Payload)
{
	// 只处理攻击命中事件
	if (!Payload.EventTag.MatchesTag(FValorisGameplayTags::Event_Attack_Hit))
	{
		return;
	}

	const AActor* TargetActor = GetCachedTarget();
	if (!TargetActor)
	{
		return;
	}

	// 获取目标的 ASC
	UAbilitySystemComponent* TargetASC = nullptr;
	if (const AValorisCharacterBase* TargetCharacter = Cast<AValorisCharacterBase>(TargetActor))
	{
		TargetASC = TargetCharacter->GetAbilitySystemComponent();
	}

	if (!TargetASC)
	{
		return;
	}

	// 应用伤害效果
	if (DamageEffect)
	{
		FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, GetAbilityLevel());
		if (SpecHandle.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
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
