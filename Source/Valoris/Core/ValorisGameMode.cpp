// Copyright Valoris. All Rights Reserved.

#include "ValorisGameMode.h"
#include "ValorisPlayerController.h"
#include "../Camera/ValorisSpectatorPawn.h"
#include "../Enemy/EnemyBase.h"
#include "../Enemy/EnemyPath.h"
#include "../Data/WaveData.h"
#include "../Economy/ResourceManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AValorisGameMode::AValorisGameMode()
{
	PlayerControllerClass = AValorisPlayerController::StaticClass();
	DefaultPawnClass = AValorisSpectatorPawn::StaticClass();

	// 创建资源管理器组件
	ResourceManager = CreateDefaultSubobject<UResourceManager>(TEXT("ResourceManager"));
}

void AValorisGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 初始化基地生命值
	BaseHealth = BaseMaxHealth;

	// 如果没有配置 EnemyPath，尝试从场景中查找
	if (!EnemyPath)
	{
		AActor* FoundPath = UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyPath::StaticClass());
		EnemyPath = Cast<AEnemyPath>(FoundPath);
	}

	// TODO: 后续改为 UI 触发，目前自动开始波次用于测试
	StartWaves();
}

void AValorisGameMode::StartWaves()
{
	if (!WaveData || WaveData->GetWaveCount() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValorisGameMode: No WaveData configured!"));
		return;
	}

	if (!EnemyPath)
	{
		UE_LOG(LogTemp, Warning, TEXT("ValorisGameMode: No EnemyPath found!"));
		return;
	}

	CurrentWaveIndex = -1;
	StartNextWave();
}

void AValorisGameMode::StartNextWave()
{
	CurrentWaveIndex++;

	if (CurrentWaveIndex >= WaveData->GetWaveCount())
	{
		// 所有波次完成
		OnAllWavesCompleted.Broadcast();
		return;
	}

	// 重置生成状态
	CurrentEnemyGroupIndex = 0;
	SpawnedInCurrentGroup = 0;
	bIsSpawning = true;

	// 广播波次开始事件
	OnWaveStarted.Broadcast(CurrentWaveIndex);

	// 开始生成敌人
	SpawnNextEnemy();
}

int32 AValorisGameMode::GetTotalWaves() const
{
	return WaveData ? WaveData->GetWaveCount() : 0;
}

bool AValorisGameMode::AreAllWavesCompleted() const
{
	return WaveData && CurrentWaveIndex >= WaveData->GetWaveCount();
}

void AValorisGameMode::SpawnEnemy(TSubclassOf<AEnemyBase> EnemyClass)
{
	if (!EnemyClass || !EnemyPath)
	{
		return;
	}

	// 在路径起点生成敌人
	FVector SpawnLocation = EnemyPath->GetLocationAtDistance(0.f);
	FRotator SpawnRotation = EnemyPath->GetRotationAtDistance(0.f);

	// 稍微抬高一点，避免卡地面
	SpawnLocation.Z += 100.f;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyBase* Enemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (Enemy)
	{
		// 设置路径
		Enemy->SetPath(EnemyPath);

		// 监听敌人销毁事件
		Enemy->OnDestroyed.AddDynamic(this, &AValorisGameMode::OnEnemyDestroyed);

		AliveEnemyCount++;
	}
}

void AValorisGameMode::SpawnNextEnemy()
{
	if (!WaveData || !bIsSpawning)
	{
		return;
	}

	const FWaveInfo& CurrentWave = WaveData->GetWaveInfo(CurrentWaveIndex);

	// 检查是否还有敌人组要生成
	if (CurrentEnemyGroupIndex >= CurrentWave.Enemies.Num())
	{
		// 当前波次所有敌人已安排生成
		bIsSpawning = false;
		return;
	}

	const FEnemySpawnInfo& CurrentGroup = CurrentWave.Enemies[CurrentEnemyGroupIndex];

	// 生成一个敌人
	SpawnEnemy(CurrentGroup.EnemyClass);
	SpawnedInCurrentGroup++;

	// 记录当前组的间隔（用于切换组时）
	float CurrentInterval = CurrentGroup.SpawnInterval;

	// 检查当前组是否生成完毕
	if (SpawnedInCurrentGroup >= CurrentGroup.Count)
	{
		// 移动到下一组
		CurrentEnemyGroupIndex++;
		SpawnedInCurrentGroup = 0;

		// 检查是否还有更多组
		if (CurrentEnemyGroupIndex >= CurrentWave.Enemies.Num())
		{
			bIsSpawning = false;
			return;
		}
	}

	// 设置定时器生成下一个敌人
	// 如果刚切换到新组，使用上一组的间隔；否则使用当前组的间隔
	float Interval = (SpawnedInCurrentGroup == 0) ? CurrentInterval : CurrentWave.Enemies[CurrentEnemyGroupIndex].SpawnInterval;

	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AValorisGameMode::SpawnNextEnemy,
		Interval,
		false
	);
}

void AValorisGameMode::CheckWaveCompletion()
{
	// 波次完成条件：所有敌人已生成且全部被消灭
	if (!bIsSpawning && AliveEnemyCount == 0)
	{
		// 广播波次完成事件
		OnWaveCompleted.Broadcast(CurrentWaveIndex);

		// 检查是否还有更多波次
		if (CurrentWaveIndex + 1 < WaveData->GetWaveCount())
		{
			// 延迟后开始下一波
			const FWaveInfo& CurrentWave = WaveData->GetWaveInfo(CurrentWaveIndex);
			GetWorld()->GetTimerManager().SetTimer(
				WaveDelayTimerHandle,
				this,
				&AValorisGameMode::StartNextWave,
				CurrentWave.DelayAfterWave,
				false
			);
		}
		else
		{
			// 所有波次完成 - 胜利
			OnAllWavesCompleted.Broadcast();

			if (!bGameOver)
			{
				bGameOver = true;
				OnGameOver.Broadcast(true); // 胜利
				UE_LOG(LogTemp, Warning, TEXT("Game Over - Victory!"));
			}
		}
	}
}

void AValorisGameMode::OnEnemyDestroyed(AActor* DestroyedActor)
{
	AliveEnemyCount = FMath::Max(0, AliveEnemyCount - 1);

	// 如果敌人是被击杀的，给予金币奖励
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(DestroyedActor))
	{
		if (Enemy->WasKilled() && ResourceManager)
		{
			ResourceManager->AddGold(Enemy->GetGoldReward());
		}
	}

	CheckWaveCompletion();
}

void AValorisGameMode::DamageBase(float Damage)
{
	if (bGameOver || Damage <= 0.f)
	{
		return;
	}

	BaseHealth = FMath::Max(0.f, BaseHealth - Damage);
	OnBaseHealthChanged.Broadcast(BaseHealth, BaseMaxHealth);

	UE_LOG(LogTemp, Log, TEXT("Base damaged: %.0f, Remaining: %.0f/%.0f"), Damage, BaseHealth, BaseMaxHealth);

	// 检查是否失败
	if (BaseHealth <= 0.f)
	{
		bGameOver = true;
		OnGameOver.Broadcast(false); // 失败

		// 停止波次生成
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
		GetWorld()->GetTimerManager().ClearTimer(WaveDelayTimerHandle);

		UE_LOG(LogTemp, Warning, TEXT("Game Over - Defeat!"));
	}
}
