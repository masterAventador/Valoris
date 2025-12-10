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

	// 初始化波次显示
	UpdateWaveDisplay(GameMode->GetCurrentWaveIndex() + 1, GameMode->GetTotalWaves());
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
