// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ValorisGameplayAbility.h"
#include "GA_TowerAttack.generated.h"

/**
 * 防御塔攻击技能
 * 通过事件触发，对目标造成伤害
 */
UCLASS()
class VALORIS_API UGA_TowerAttack : public UValorisGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_TowerAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	// 获取冷却时间
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
