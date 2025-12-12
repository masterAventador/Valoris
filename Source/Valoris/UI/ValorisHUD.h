// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ValorisHUD.generated.h"

class UTextBlock;
class UResourceManager;

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

	// 更新金币显示
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateGoldDisplay(int32 NewGold);

	// 更新波次显示
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateWaveDisplay(int32 CurrentWave, int32 TotalWaves);

	// 更新基地生命值显示
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateBaseHealthDisplay(float CurrentHealth, float MaxHealth);

	// 显示游戏结果
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void ShowGameResult(bool bVictory);

protected:
	virtual void NativeConstruct() override;

	// 金币变化回调
	UFUNCTION()
	void OnGoldChanged(int32 NewGold, int32 Delta);

	// 波次开始回调
	UFUNCTION()
	void OnWaveStarted(int32 WaveIndex);

	// 所有波次完成回调
	UFUNCTION()
	void OnAllWavesCompleted();

	// 基地生命值变化回调
	UFUNCTION()
	void OnBaseHealthChanged(float NewHealth, float MaxHealth);

	// 游戏结束回调
	UFUNCTION()
	void OnGameOver(bool bVictory);

	//~ UI 组件（在蓝图中绑定）

	// 金币文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldText;

	// 波次文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WaveText;

	// 基地生命值文本
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BaseHealthText;

	// 游戏结果文本
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> GameResultText;
};
