// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WaveData.generated.h"

class AEnemyBase;

/**
 * 单个敌人生成配置
 */
USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
	GENERATED_BODY()

	// 敌人类型
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AEnemyBase> EnemyClass;

	// 生成数量
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1"))
	int32 Count = 1;

	// 每个敌人之间的生成间隔（秒）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.1"))
	float SpawnInterval = 1.0f;
};

/**
 * 单波敌人配置
 */
USTRUCT(BlueprintType)
struct FWaveInfo
{
	GENERATED_BODY()

	// 波次名称（可选，用于 UI 显示）
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText WaveName;

	// 该波次的敌人配置
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FEnemySpawnInfo> Enemies;

	// 该波次完成后到下一波的等待时间（秒）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	float DelayAfterWave = 5.0f;

	// 完成该波次的奖励金币
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32 GoldReward = 50;
};

/**
 * 波次数据资产
 * 用于配置一个关卡的所有波次
 */
UCLASS(BlueprintType)
class VALORIS_API UWaveData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 所有波次配置
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Waves")
	TArray<FWaveInfo> Waves;

	// 获取波次数量
	UFUNCTION(BlueprintCallable, Category = "Waves")
	int32 GetWaveCount() const { return Waves.Num(); }

	// 获取指定波次信息
	UFUNCTION(BlueprintCallable, Category = "Waves")
	const FWaveInfo& GetWaveInfo(int32 WaveIndex) const;
};
