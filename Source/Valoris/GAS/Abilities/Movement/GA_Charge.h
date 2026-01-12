// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../../GA_MontageAbilityBase.h"
#include "GA_Charge.generated.h"

/**
 * 冲锋技能
 * 向目标方向快速移动，撞击敌人造成伤害
 */
UCLASS()
class VALORIS_API UGA_Charge : public UGA_MontageAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Charge();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	virtual void OnEventReceived(FGameplayEventData Payload) override;

	// ========== 移动配置 ==========

	/** 冲锋距离 */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float ChargeDistance = 800.0f;

	/** 冲锋速度 */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float ChargeSpeed = 2000.0f;

	/** 冲锋持续时间（根据距离和速度自动计算，也可手动覆盖） */
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float ChargeDuration = 0.0f;

	// ========== 伤害配置 ==========

	/** 撞击伤害效果 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> ImpactDamageEffect;

	/** 伤害倍率（基于 AttackPower） */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageMultiplier = 1.5f;

	/** 撞击检测半径 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float ImpactRadius = 100.0f;

private:
	/** 冲锋方向 */
	FVector ChargeDirection;

	/** 冲锋起始位置 */
	FVector ChargeStartLocation;

	/** 已冲锋距离 */
	float ChargedDistance;

	/** 执行冲锋移动 */
	void PerformCharge();

	/** 检测并处理撞击 */
	void CheckImpact();

	/** 对目标应用撞击伤害 */
	void ApplyImpactDamage(AActor* Target);

	/** 已经撞击过的目标（避免重复伤害） */
	TArray<TWeakObjectPtr<AActor>> HitTargets;

	/** 冲锋定时器句柄 */
	FTimerHandle ChargeTimerHandle;

	/** 冲锋结束 */
	void OnChargeEnd();
};
