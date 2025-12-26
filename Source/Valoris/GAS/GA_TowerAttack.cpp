// Copyright Valoris. All Rights Reserved.

#include "GA_TowerAttack.h"
#include "AbilitySystemComponent.h"
#include "../Tower/TowerBase.h"
#include "../Character/ValorisCharacterBase.h"
#include "../Enemy/EnemyBase.h"
#include "ValorisAttributeSet.h"
#include "ValorisGameplayTags.h"
#include "GE_Cooldown_Attack.h"
#include "GE_Damage.h"

UGA_TowerAttack::UGA_TowerAttack()
{
	// 冷却 Tag（CooldownGameplayEffectClass 在蓝图中配置）
	CooldownTags.AddTag(FValorisGameplayTags::Cooldown_Attack);

	// 设置伤害效果
	DamageEffect = UGE_Damage::StaticClass();
}

void UGA_TowerAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 从塔获取当前攻击目标
	ATowerBase* Tower = Cast<ATowerBase>(GetAvatarActorFromActorInfo());
	if (!Tower)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	AActor* TargetActor = Tower->GetCurrentTarget();
	if (!TargetActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// 获取目标的 ASC（敌人是 ValorisCharacterBase 的子类）
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

	// 应用伤害效果
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

const FGameplayTagContainer* UGA_TowerAttack::GetCooldownTags() const
{
	return &CooldownTags;
}

void UGA_TowerAttack::ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo) const
{
	if (!CooldownGameplayEffectClass)
	{
		return;
	}

	// 计算冷却时间（基于攻速）
	float CooldownDuration = 1.0f;
	if (ATowerBase* Tower = Cast<ATowerBase>(ActorInfo->AvatarActor.Get()))
	{
		if (UValorisAttributeSet* Attributes = Tower->GetAttributeSet())
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
