// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../../GA_MontageAbilityBase.h"
#include "GA_Dodge.generated.h"

/**
 * 闪避翻滚技能
 * 朝玩家移动输入方向快速位移，期间挂 State.Invincible 无敌帧，无伤害。
 */
UCLASS()
class VALORIS_API UGA_Dodge : public UGA_MontageAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Dodge();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void OnEventReceived(FGameplayEventData Payload) override;

	/** 翻滚距离 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float DodgeDistance = 500.0f;

	/** 翻滚速度 */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float DodgeSpeed = 2000.0f;

	/** 翻滚持续时间（<=0 则按距离/速度自动计算） */
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float DodgeDuration = 0.0f;

private:
	/** 翻滚方向 */
	FVector DodgeDirection;

	/** 已位移距离 */
	float DodgedDistance;

	/** 执行翻滚位移 */
	void PerformDodge();

	/** 翻滚定时器句柄 */
	FTimerHandle DodgeTimerHandle;

	/** 翻滚结束 */
	void OnDodgeEnd();
};
