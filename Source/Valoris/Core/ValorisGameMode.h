// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ValorisGameMode.generated.h"

class UWaveData;
class AEnemyBase;
class AEnemyPath;
class UResourceManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStarted, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveCompleted, int32, WaveIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllWavesCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBaseHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameOver, bool, bVictory);

/**
 * 游戏模式基类
 * 负责游戏流程控制、波次管理、胜负判定
 */
UCLASS()
class VALORIS_API AValorisGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AValorisGameMode();

	//~ 资源管理

	// 获取资源管理器
	UFUNCTION(BlueprintCallable, Category = "Resource")
	UResourceManager* GetResourceManager() const { return ResourceManager; }

	//~ 波次管理

	// 开始波次系统
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWaves();

	// 开始下一波
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartNextWave();

	// 获取当前波次索引（从0开始）
	UFUNCTION(BlueprintCallable, Category = "Wave")
	int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

	// 获取总波次数
	UFUNCTION(BlueprintCallable, Category = "Wave")
	int32 GetTotalWaves() const;

	// 是否所有波次已完成
	UFUNCTION(BlueprintCallable, Category = "Wave")
	bool AreAllWavesCompleted() const;

	//~ 事件

	// 波次开始事件
	UPROPERTY(BlueprintAssignable, Category = "Wave|Events")
	FOnWaveStarted OnWaveStarted;

	// 波次完成事件
	UPROPERTY(BlueprintAssignable, Category = "Wave|Events")
	FOnWaveCompleted OnWaveCompleted;

	// 所有波次完成事件
	UPROPERTY(BlueprintAssignable, Category = "Wave|Events")
	FOnAllWavesCompleted OnAllWavesCompleted;

	//~ 基地系统

	// 获取基地当前生命值
	UFUNCTION(BlueprintCallable, Category = "Base")
	float GetBaseHealth() const { return BaseHealth; }

	// 获取基地最大生命值
	UFUNCTION(BlueprintCallable, Category = "Base")
	float GetBaseMaxHealth() const { return BaseMaxHealth; }

	// 对基地造成伤害
	UFUNCTION(BlueprintCallable, Category = "Base")
	void DamageBase(float Damage);

	// 基地生命值变化事件
	UPROPERTY(BlueprintAssignable, Category = "Base|Events")
	FOnBaseHealthChanged OnBaseHealthChanged;

	// 游戏结束事件（胜利/失败）
	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FOnGameOver OnGameOver;

	// 游戏是否已结束
	UFUNCTION(BlueprintCallable, Category = "Game")
	bool IsGameOver() const { return bGameOver; }

protected:
	virtual void BeginPlay() override;

	//~ 波次内部逻辑

	// 生成单个敌人
	void SpawnEnemy(TSubclassOf<AEnemyBase> EnemyClass);

	// 生成下一个敌人（定时器回调）
	void SpawnNextEnemy();

	// 检查当前波次是否完成
	void CheckWaveCompletion();

	// 敌人死亡回调
	UFUNCTION()
	void OnEnemyDestroyed(AActor* DestroyedActor);

	//~ 配置

	// 波次数据（在蓝图中配置）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wave|Config")
	TObjectPtr<UWaveData> WaveData;

	// 敌人移动路径（在蓝图中配置，或运行时查找）
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Wave|Config")
	TObjectPtr<AEnemyPath> EnemyPath;

	//~ 运行时状态

	// 当前波次索引
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave|State")
	int32 CurrentWaveIndex = -1;

	// 当前波次中正在生成的敌人组索引
	int32 CurrentEnemyGroupIndex = 0;

	// 当前敌人组中已生成的数量
	int32 SpawnedInCurrentGroup = 0;

	// 当前波次存活的敌人数量
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Wave|State")
	int32 AliveEnemyCount = 0;

	// 当前波次是否正在生成敌人
	bool bIsSpawning = false;

	// 生成定时器
	FTimerHandle SpawnTimerHandle;

	// 波次间隔定时器
	FTimerHandle WaveDelayTimerHandle;

	//~ 资源

	// 资源管理器组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Resource")
	TObjectPtr<UResourceManager> ResourceManager;

	//~ 基地

	// 基地最大生命值
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Base|Config")
	float BaseMaxHealth = 20.f;

	// 基地当前生命值
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Base|State")
	float BaseHealth = 20.f;

	// 游戏是否已结束
	bool bGameOver = false;
};
