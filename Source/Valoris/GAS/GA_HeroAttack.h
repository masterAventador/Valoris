// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ValorisGameplayAbility.h"
#include "GA_HeroAttack.generated.h"

/**
 * 英雄普通攻击技能
 */
UCLASS()
class VALORIS_API UGA_HeroAttack : public UValorisGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_HeroAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 获取冷却时间（根据攻速动态计算）
	virtual const FGameplayTagContainer* GetCooldownTags() const override;
	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

protected:
	// 伤害效果
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffect;

private:
	// 冷却 Tag 容器
	UPROPERTY()
	FGameplayTagContainer CooldownTags;
};
