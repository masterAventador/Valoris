// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../../GA_MontageAbilityBase.h"
#include "GA_MeleeAttack.generated.h"

/**
 * 近战单体攻击技能
 * 对单个目标造成伤害，可选附加控制效果（如眩晕）
 */
UCLASS()
class VALORIS_API UGA_MeleeAttack : public UGA_MontageAbilityBase
{
	GENERATED_BODY()

public:
	UGA_MeleeAttack();

protected:
	virtual void OnEventReceived(FGameplayEventData Payload) override;

	// ========== 伤害配置 ==========

	/** 伤害效果 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** 伤害倍率（基于 AttackPower） */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageMultiplier = 1.0f;

	// ========== 范围配置 ==========

	/** 攻击范围 */
	UPROPERTY(EditDefaultsOnly, Category = "Range")
	float AttackRange = 200.0f;

	// ========== 控制效果配置（可选） ==========

	/** 控制效果（如眩晕），为空则不应用 */
	UPROPERTY(EditDefaultsOnly, Category = "Control")
	TSubclassOf<UGameplayEffect> ControlEffect;

private:
	/** 对目标应用伤害和控制效果 */
	void ApplyDamageToTarget(AActor* Target);
};
