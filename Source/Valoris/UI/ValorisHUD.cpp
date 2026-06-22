// Copyright Valoris. All Rights Reserved.

#include "ValorisHUD.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "../Core/ValorisGameMode.h"
#include "../GAS/ValorisAttributeSet.h"

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

	// 绑定波次事件
	GameMode->OnWaveStarted.AddDynamic(this, &UValorisHUD::OnWaveStarted);
	GameMode->OnAllWavesCompleted.AddDynamic(this, &UValorisHUD::OnAllWavesCompleted);

	// 绑定游戏结束事件
	GameMode->OnGameOver.AddDynamic(this, &UValorisHUD::OnGameOver);

	// 剩余敌人数
	GameMode->OnEnemyCountChanged.AddDynamic(this, &UValorisHUD::OnEnemyCountChanged);
	OnEnemyCountChanged(GameMode->GetAliveEnemyCount());

	// 玩家血条：绑玩家 ASC 的 Health 变化
	if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn))
		{
			ASC->GetGameplayAttributeValueChangeDelegate(UValorisAttributeSet::GetHealthAttribute())
				.AddUObject(this, &UValorisHUD::OnPlayerHealthChanged);
			RefreshPlayerHealth();
		}
	}

	// 初始化波次显示
	UpdateWaveDisplay(GameMode->GetCurrentWaveIndex() + 1, GameMode->GetTotalWaves());

	// 隐藏波次横幅初始
	if (WaveBannerText)
	{
		WaveBannerText->SetVisibility(ESlateVisibility::Hidden);
	}

	// 隐藏结算面板 + 绑重开按钮
	if (GameOverPanel)
	{
		GameOverPanel->SetVisibility(ESlateVisibility::Hidden);
	}
	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UValorisHUD::OnRestartClicked);
	}
}

void UValorisHUD::UpdateWaveDisplay(int32 CurrentWave, int32 TotalWaves)
{
	if (WaveText)
	{
		WaveText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentWave, TotalWaves)));
	}
}

void UValorisHUD::OnEnemyCountChanged(int32 NewCount)
{
	if (RemainingEnemiesText)
	{
		RemainingEnemiesText->SetText(FText::FromString(FString::Printf(TEXT("%d"), NewCount)));
	}
}

void UValorisHUD::OnPlayerHealthChanged(const FOnAttributeChangeData& Data)
{
	RefreshPlayerHealth();
}

void UValorisHUD::RefreshPlayerHealth()
{
	if (!PlayerHealthBar)
	{
		return;
	}
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	UAbilitySystemComponent* ASC = PlayerPawn ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn) : nullptr;
	if (!ASC)
	{
		return;
	}
	const float H = ASC->GetNumericAttribute(UValorisAttributeSet::GetHealthAttribute());
	const float MaxH = ASC->GetNumericAttribute(UValorisAttributeSet::GetMaxHealthAttribute());
	PlayerHealthBar->SetPercent(MaxH > 0.f ? H / MaxH : 0.f);
}

void UValorisHUD::OnWaveStarted(int32 WaveIndex)
{
	AValorisGameMode* GameMode = Cast<AValorisGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		// WaveIndex 是从 0 开始的，显示时 +1
		UpdateWaveDisplay(WaveIndex + 1, GameMode->GetTotalWaves());
	}

	if (WaveBannerText)
	{
		WaveBannerText->SetText(FText::FromString(FString::Printf(TEXT("第 %d 波"), WaveIndex + 1)));
		WaveBannerText->SetVisibility(ESlateVisibility::HitTestInvisible);
		FTimerHandle BannerTimer;
		GetWorld()->GetTimerManager().SetTimer(BannerTimer, [this]()
		{
			if (WaveBannerText) { WaveBannerText->SetVisibility(ESlateVisibility::Hidden); }
		}, 2.0f, false);
	}
}

void UValorisHUD::OnAllWavesCompleted()
{
	if (WaveText)
	{
		WaveText->SetText(FText::FromString(TEXT("Complete!")));
	}
}

void UValorisHUD::OnGameOver(bool bVictory)
{
	if (GameOverResultText)
	{
		GameOverResultText->SetText(FText::FromString(bVictory ? TEXT("胜利！") : TEXT("失败")));
	}
	if (GameOverWavesText)
	{
		AValorisGameMode* GameMode = Cast<AValorisGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		const int32 Waves = GameMode ? (GameMode->GetCurrentWaveIndex() + 1) : 0;
		GameOverWavesText->SetText(FText::FromString(FString::Printf(TEXT("撑过 %d 波"), Waves)));
	}
	if (GameOverPanel)
	{
		GameOverPanel->SetVisibility(ESlateVisibility::Visible);
	}

	// 暂停 + 解锁鼠标，便于点重开
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->SetPause(true);
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());
	}
}

void UValorisHUD::OnRestartClicked()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->SetPause(false);
	}
	const FName CurrentLevel(*UGameplayStatics::GetCurrentLevelName(GetWorld(), true));
	UGameplayStatics::OpenLevel(GetWorld(), CurrentLevel);
}
