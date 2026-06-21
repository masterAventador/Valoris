// Copyright Valoris. All Rights Reserved.

#include "EnemyBase.h"
#include "EnemyAIController.h"
#include "EnemyPath.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "../GAS/ValorisAttributeSet.h"
#include "../Core/ValorisGameMode.h"
#include "../UI/HealthBarWidget.h"
#include "Kismet/GameplayStatics.h"

AEnemyBase::AEnemyBase()
{
	// 敌人使用自己的 AIController
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 敌人需要 Tick 来沿路径移动
	PrimaryActorTick.bCanEverTick = true;

	// 创建血条组件
	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawAtDesiredSize(true);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// 初始化血条
	InitializeHealthBar();
}

void AEnemyBase::InitializeHealthBar()
{
	if (!HealthBarComponent || !HealthBarWidgetClass)
	{
		return;
	}

	// 设置血条位置
	HealthBarComponent->SetRelativeLocation(FVector(0.f, 0.f, HealthBarHeight));

	// 设置 Widget 类
	HealthBarComponent->SetWidgetClass(HealthBarWidgetClass);

	// 绑定到 ASC
	if (UHealthBarWidget* HealthBarWidget = Cast<UHealthBarWidget>(HealthBarComponent->GetWidget()))
	{
		HealthBarWidget->BindToASC(AbilitySystemComponent);
	}
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// M1：朝玩家移动（平地竞技场，不依赖 NavMesh，直接 AddMovementInput）
	// M2 接回沿 Spline/正式波次时再按需切换
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}

	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const float Distance = ToPlayer.Size();

	// 离玩家还远就继续靠近（120 以内算贴身，M1 只贴着不攻击）
	if (Distance > 120.f)
	{
		const FVector Dir = ToPlayer.GetSafeNormal();
		AddMovementInput(Dir, 1.f);
		SetActorRotation(FRotator(0.f, Dir.Rotation().Yaw, 0.f));
	}
}

void AEnemyBase::SetPath(AEnemyPath* InPath)
{
	CurrentPath = InPath;
	CurrentDistance = 0.f;

	// 将敌人放到路径起点
	if (CurrentPath)
	{
		FVector StartLocation = CurrentPath->GetLocationAtDistance(0.f);
		FRotator StartRotation = CurrentPath->GetRotationAtDistance(0.f);

		StartLocation.Z = GetActorLocation().Z;
		SetActorLocation(StartLocation);
		SetActorRotation(StartRotation);
	}
}

void AEnemyBase::OnReachedEnd_Implementation()
{
	// 竞技场不再有"到达基地"概念（敌人直接追玩家）。路径系统属塔防遗留，留待清理里程碑。
	Destroy();
}
