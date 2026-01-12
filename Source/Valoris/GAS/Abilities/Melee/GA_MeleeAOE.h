// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../../GA_MontageAbilityBase.h"
#include "GA_MeleeAOE.generated.h"

/**
 * 近战范围攻击技能
 * 对周围范围内的多个目标造成伤害
 */
UCLASS()
class VALORIS_API UGA_MeleeAOE : public UGA_MontageAbilityBase
{
	GENERATED_BODY()

public:
	UGA_MeleeAOE();

protected:
	virtual void OnEventReceived(FGameplayEventData Payload) override;

	// ========== 伤害配置 ==========

	/** 伤害效果 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffect;

	/** 伤害倍率（基于 AttackPower） */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageMultiplier = 1.0f;

	// ========== AOE 配置 ==========

	/** AOE 半径 */
	UPROPERTY(EditDefaultsOnly, Category = "AOE")
	float AOERadius = 300.0f;

	/** 最大目标数量（0 = 无限制） */
	UPROPERTY(EditDefaultsOnly, Category = "AOE")
	int32 MaxTargets = 0;

	/** 目标碰撞通道 */
	UPROPERTY(EditDefaultsOnly, Category = "AOE")
	TEnumAsByte<ECollisionChannel> TargetChannel = ECC_Pawn;

private:
	/** 查找 AOE 范围内的目标并应用伤害 */
	void ApplyAOEDamage();

	/** 对单个目标应用伤害 */
	void ApplyDamageToTarget(AActor* Target, float Damage);
};
