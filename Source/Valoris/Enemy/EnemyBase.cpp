// Copyright Valoris. All Rights Reserved.

#include "EnemyBase.h"
#include "EnemyAIController.h"
#include "EnemyPath.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "../GAS/ValorisAttributeSet.h"
#include "../Core/ValorisGameMode.h"
#include "../UI/HealthBarWidget.h"

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

	// 如果没有路径，不移动
	if (!CurrentPath)
	{
		return;
	}

	// 获取移动速度
	float MoveSpeed = 300.f; // 默认速度
	if (AttributeSet)
	{
		MoveSpeed = AttributeSet->GetMoveSpeed();
	}

	// 更新路径上的距离
	CurrentDistance += MoveSpeed * DeltaTime;

	// 检查是否到达终点
	float PathLength = CurrentPath->GetPathLength();
	if (CurrentDistance >= PathLength)
	{
		OnReachedEnd();
		return;
	}

	// 获取当前位置和旋转
	FVector NewLocation = CurrentPath->GetLocationAtDistance(CurrentDistance);
	FRotator NewRotation = CurrentPath->GetRotationAtDistance(CurrentDistance);

	// 设置位置（保持 Z 轴不变，使用地面高度）
	NewLocation.Z = GetActorLocation().Z;
	SetActorLocation(NewLocation);
	SetActorRotation(NewRotation);
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
	// 对基地造成伤害
	if (AValorisGameMode* GameMode = Cast<AValorisGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->DamageBase(BaseDamage);
	}

	// 销毁敌人（不标记为击杀，不给奖励）
	Destroy();
}
