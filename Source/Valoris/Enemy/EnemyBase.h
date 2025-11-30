// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Character/ValorisCharacterBase.h"
#include "EnemyBase.generated.h"

/**
 * 敌人基类
 * 所有敌人类型（普通、快速、坦克、飞行、Boss）都继承自此类
 */
UCLASS()
class VALORIS_API AEnemyBase : public AValorisCharacterBase
{
	GENERATED_BODY()

public:
	AEnemyBase();

	// 获取敌人对基地造成的伤害
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	float GetBaseDamage() const { return BaseDamage; }

	// 获取击杀奖励金币
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	int32 GetGoldReward() const { return GoldReward; }

protected:
	// 到达基地时对基地造成的伤害
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float BaseDamage = 1.f;

	// 击杀奖励金币
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	int32 GoldReward = 10;
};
