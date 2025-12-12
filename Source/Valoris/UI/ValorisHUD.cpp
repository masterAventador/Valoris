// Copyright Valoris. All Rights Reserved.

#include "ValorisHUD.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "../Core/ValorisGameMode.h"
#include "../Economy/ResourceManager.h"

void UValorisHUD::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeHUD();
}

void UValorisHUD::InitializeHUD()
{
	AValorisGameMode* GameMode = Cast<AValorisGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!GameMode)
	{
		return;
	}

	// 绑定资源管理器事件
	if (UResourceManager* ResourceManager = GameMode->GetResourceManager())
	{
		ResourceManager->OnGoldChanged.AddDynamic(this, &UValorisHUD::OnGoldChanged);

		// 初始化显示
		UpdateGoldDisplay(ResourceManager->GetGold());
	}

	// 绑定波次事件
	GameMode->OnWaveStarted.AddDynamic(this, &UValorisHUD::OnWaveStarted);
	GameMode->OnAllWavesCompleted.AddDynamic(this, &UValorisHUD::OnAllWavesCompleted);

	// 绑定基地生命值事件
	GameMode->OnBaseHealthChanged.AddDynamic(this, &UValorisHUD::OnBaseHealthChanged);

	// 绑定游戏结束事件
	GameMode->OnGameOver.AddDynamic(this, &UValorisHUD::OnGameOver);

	// 初始化波次显示
	UpdateWaveDisplay(GameMode->GetCurrentWaveIndex() + 1, GameMode->GetTotalWaves());

	// 初始化基地生命值显示
	UpdateBaseHealthDisplay(GameMode->GetBaseHealth(), GameMode->GetBaseMaxHealth());

	// 隐藏游戏结果文本
	if (GameResultText)
	{
		GameResultText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UValorisHUD::UpdateGoldDisplay(int32 NewGold)
{
	if (GoldText)
	{
		GoldText->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewGold)));
	}
}

void UValorisHUD::UpdateWaveDisplay(int32 CurrentWave, int32 TotalWaves)
{
	if (WaveText)
	{
		WaveText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentWave, TotalWaves)));
	}
}

void UValorisHUD::OnGoldChanged(int32 NewGold, int32 Delta)
{
	UpdateGoldDisplay(NewGold);
}

void UValorisHUD::OnWaveStarted(int32 WaveIndex)
{
	AValorisGameMode* GameMode = Cast<AValorisGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		// WaveIndex 是从 0 开始的，显示时 +1
		UpdateWaveDisplay(WaveIndex + 1, GameMode->GetTotalWaves());
	}
}

void UValorisHUD::OnAllWavesCompleted()
{
	if (WaveText)
	{
		WaveText->SetText(FText::FromString(TEXT("Complete!")));
	}
}

void UValorisHUD::UpdateBaseHealthDisplay(float CurrentHealth, float MaxHealth)
{
	if (BaseHealthText)
	{
		BaseHealthText->SetText(FText::FromString(FString::Printf(TEXT("%.0f / %.0f"), CurrentHealth, MaxHealth)));
	}
}

void UValorisHUD::ShowGameResult(bool bVictory)
{
	if (GameResultText)
	{
		GameResultText->SetText(FText::FromString(bVictory ? TEXT("VICTORY!") : TEXT("DEFEAT!")));
		GameResultText->SetVisibility(ESlateVisibility::Visible);
	}
}

void UValorisHUD::OnBaseHealthChanged(float NewHealth, float MaxHealth)
{
	UpdateBaseHealthDisplay(NewHealth, MaxHealth);
}

void UValorisHUD::OnGameOver(bool bVictory)
{
	ShowGameResult(bVictory);
}
