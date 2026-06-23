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
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "../Combat/ArenaCombatUtils.h"
#include "../GAS/GE_Damage.h"
#include "../GAS/ValorisGameplayTags.h"

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

	MeleeDamageEffect = UGE_Damage::StaticClass();
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	// 初始化血条
	InitializeHealthBar();

	if (GetMesh())
	{
		InitialMeshScale = GetMesh()->GetRelativeScale3D();
	}
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

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player)
	{
		return;
	}

	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	const float Distance = ToPlayer.Size();
	const bool bInAttackRange = Distance <= AttackRange;

	// 仅 Approaching 且未进入攻击距离时追击移动
	if (CurrentAttackPhase == ArenaCombat::EEnemyAttackPhase::Approaching && !bInAttackRange)
	{
		const FVector Dir = ToPlayer.GetSafeNormal();
		AddMovementInput(Dir, 1.f);
		SetActorRotation(FRotator(0.f, Dir.Rotation().Yaw, 0.f));
	}

	// 推进攻击状态机
	const ArenaCombat::FEnemyAttackStep Step = ArenaCombat::StepEnemyAttack(
		CurrentAttackPhase, AttackAccumulator, DeltaTime, WindupDuration, RecoveryDuration, bInAttackRange);
	CurrentAttackPhase = Step.Phase;

	// 前摇结束的挥击
	if (Step.bStrike)
	{
		if (ArenaCombat::ShouldEnemyStrikeConnect(Distance, AttackReach, IsTargetInvincible(Player)))
		{
			ApplyMeleeDamageTo(Player);
		}
	}

	UpdateWindupVisual();
}

void AEnemyBase::ApplyMeleeDamageTo(AActor* Target)
{
	if (!Target || !MeleeDamageEffect)
	{
		return;
	}

	UAbilitySystemComponent* SelfASC = GetAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (!SelfASC || !TargetASC)
	{
		return;
	}

	FGameplayEffectContextHandle Ctx = SelfASC->MakeEffectContext();
	Ctx.AddSourceObject(this);
	FGameplayEffectSpecHandle Spec = SelfASC->MakeOutgoingSpec(MeleeDamageEffect, 1.f, Ctx);
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(FValorisGameplayTags::Data_Damage, AttackDamage);
		TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

bool AEnemyBase::IsTargetInvincible(AActor* Target) const
{
	if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
	{
		return TargetASC->HasMatchingGameplayTag(FValorisGameplayTags::State_Invincible);
	}
	return false;
}

void AEnemyBase::UpdateWindupVisual()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	if (CurrentAttackPhase == ArenaCombat::EEnemyAttackPhase::WindingUp)
	{
		const float Alpha = WindupDuration > 0.f ? FMath::Clamp(AttackAccumulator / WindupDuration, 0.f, 1.f) : 1.f;
		MeshComp->SetRelativeScale3D(InitialMeshScale * FMath::Lerp(1.f, 1.15f, Alpha));
	}
	else
	{
		MeshComp->SetRelativeScale3D(InitialMeshScale);
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
