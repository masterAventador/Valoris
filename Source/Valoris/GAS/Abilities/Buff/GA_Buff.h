// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../../GA_MontageAbilityBase.h"
#include "GA_Buff.generated.h"

/**
 * 增益技能
 * 对自身或目标施加增益效果
 */
UCLASS()
class VALORIS_API UGA_Buff : public UGA_MontageAbilityBase
{
	GENERATED_BODY()

public:
	UGA_Buff();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	virtual void OnEventReceived(FGameplayEventData Payload) override;

	// ========== 增益配置 ==========

	/** 增益效果 */
	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	TSubclassOf<UGameplayEffect> BuffEffect;

	/** 增益持续时间（覆盖效果默认时长） */
	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	float BuffDuration = 5.0f;

	/** 是否对自身施加（false 则对目标施加） */
	UPROPERTY(EditDefaultsOnly, Category = "Buff")
	bool bApplyToSelf = true;

private:
	/** 应用增益效果 */
	void ApplyBuffEffect();

	/** 是否已应用增益（避免重复应用） */
	bool bBuffApplied = false;
};
