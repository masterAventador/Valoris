// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "ValorisHUD.generated.h"

class UTextBlock;

/**
 * 游戏主 HUD
 * 显示金币、波次等信息
 */
UCLASS()
class VALORIS_API UValorisHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	// 初始化 HUD，绑定事件
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void InitializeHUD();

	// 更新波次显示
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateWaveDisplay(int32 CurrentWave, int32 TotalWaves);

	// 显示游戏结果
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowGameResult(bool bVictory);

protected:
	virtual void NativeConstruct() override;

	// 波次开始回调
	UFUNCTION()
	void OnWaveStarted(int32 WaveIndex);

	// 所有波次完成回调
	UFUNCTION()
	void OnAllWavesCompleted();

	// 游戏结束回调
	UFUNCTION()
	void OnGameOver(bool bVictory);

	// 玩家血量变化回调
	void OnPlayerHealthChanged(const struct FOnAttributeChangeData& Data);

	// 剩余敌人数变化回调
	UFUNCTION()
	void OnEnemyCountChanged(int32 NewCount);

	// 刷新玩家血条百分比
	void RefreshPlayerHealth();

	//~ UI 组件（在蓝图中绑定）

	// 玩家血条
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<class UProgressBar> PlayerHealthBar;

	// 本波剩余敌人数文本
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RemainingEnemiesText;

	// "第 N 波" 横幅文本
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WaveBannerText;

	// 波次文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WaveText;

	// 游戏结果文本
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameResultText;
};
