// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "../Character/ValorisCharacterBase.h"
#include "EnemyBase.generated.h"

class AEnemyPath;
class UWidgetComponent;
class UHealthBarWidget;

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

	// ========== 血条 ==========

	// 血条 Widget 组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	// 血条 Widget 类（在蓝图中设置）
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UHealthBarWidget> HealthBarWidgetClass;

	// 血条高度偏移
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HealthBarHeight = 120.f;

	// 初始化血条
	void InitializeHealthBar();

	// ========== 接触伤害 ==========

	// 接触伤害：贴身时按间隔对玩家施加的 GameplayEffect（默认 GE_Damage）
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	TSubclassOf<class UGameplayEffect> ContactDamageEffect;

	// 单次接触伤害值
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float ContactDamage = 5.f;

	// 接触伤害间隔（秒）
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float ContactDamageInterval = 0.5f;

	// 接触伤害节拍累加器
	float ContactDamageAccumulator = 0.f;

	// 对目标施加一次接触伤害
	void ApplyContactDamageTo(AActor* Target);
};
