// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Character/ValorisCharacterBase.h"
#include "EnemyBase.generated.h"

class AEnemyPath;

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

	virtual void Tick(float DeltaTime) override;

	// 设置移动路径
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetPath(AEnemyPath* InPath);

	// 获取当前路径
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	AEnemyPath* GetPath() const { return CurrentPath; }

	// 获取当前在路径上的距离
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	float GetCurrentDistance() const { return CurrentDistance; }

	// 获取敌人对基地造成的伤害
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	float GetBaseDamage() const { return BaseDamage; }

	// 获取击杀奖励金币
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	int32 GetGoldReward() const { return GoldReward; }

	// 是否被击杀（用于区分击杀和到达终点）
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	bool WasKilled() const { return bWasKilled; }

	// 标记为被击杀
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void MarkAsKilled() { bWasKilled = true; }

protected:
	virtual void BeginPlay() override;

	// 到达终点时调用
	UFUNCTION(BlueprintNativeEvent, Category = "Enemy")
	void OnReachedEnd();
	virtual void OnReachedEnd_Implementation();

	// 当前路径
	UPROPERTY(EditInstanceOnly, Category = "Enemy|Path")
	TObjectPtr<AEnemyPath> CurrentPath;

	// 当前在路径上的距离
	UPROPERTY(VisibleInstanceOnly, Category = "Enemy|Path")
	float CurrentDistance = 0.f;

	// 到达基地时对基地造成的伤害
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	float BaseDamage = 1.f;

	// 击杀奖励金币
	UPROPERTY(EditDefaultsOnly, Category = "Enemy")
	int32 GoldReward = 10;

	// 是否被击杀（区分死亡原因）
	bool bWasKilled = false;
};
