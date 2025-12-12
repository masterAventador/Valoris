// Copyright Valoris. All Rights Reserved.

#include "EnemyBase.h"
#include "EnemyAIController.h"
#include "EnemyPath.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../GAS/ValorisAttributeSet.h"
#include "../Core/ValorisGameMode.h"

AEnemyBase::AEnemyBase()
{
	// 敌人使用自己的 AIController
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 敌人需要 Tick 来沿路径移动
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
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
