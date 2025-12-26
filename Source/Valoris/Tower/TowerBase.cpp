// Copyright Valoris. All Rights Reserved.

#include "TowerBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "../GAS/ValorisAttributeSet.h"
#include "../GAS/ValorisGameplayTags.h"
#include "../GAS/GA_TowerAttack.h"
#include "../Enemy/EnemyBase.h"
#include "Kismet/GameplayStatics.h"

ATowerBase::ATowerBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// 创建网格体组件
	TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
	TowerMesh->SetupAttachment(RootComponent);

	// 创建 ASC
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// 创建属性集
	AttributeSet = CreateDefaultSubobject<UValorisAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ATowerBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ATowerBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilitySystem();
}

void ATowerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 自动索敌并攻击
	TryAttackTarget();
}

void ATowerBase::InitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 初始化 ASC
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// 应用默认属性
	ApplyDefaultAttributes();

	// 授予默认技能
	GiveDefaultAbilities();
}

void ATowerBase::GiveDefaultAbilities()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClass, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}

void ATowerBase::ApplyDefaultAttributes()
{
	if (!AbilitySystemComponent || !DefaultAttributeEffect)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
		DefaultAttributeEffect, 1, EffectContext);

	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

AEnemyBase* ATowerBase::FindTargetEnemy() const
{
	if (!AttributeSet)
	{
		return nullptr;
	}

	const float AttackRange = AttributeSet->GetAttackRange();
	const FVector TowerLocation = GetActorLocation();

	// 获取所有敌人
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundActors);

	AEnemyBase* ClosestEnemy = nullptr;
	float ClosestDistance = AttackRange;

	for (AActor* Actor : FoundActors)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(Actor);
		if (!Enemy)
		{
			continue;
		}

		// 检查敌人是否存活
		if (UValorisAttributeSet* EnemyAttributes = Enemy->GetAttributeSet())
		{
			if (EnemyAttributes->GetHealth() <= 0.f)
			{
				continue;
			}
		}

		// 计算距离
		const float Distance = FVector::Dist(TowerLocation, Enemy->GetActorLocation());
		if (Distance <= ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestEnemy = Enemy;
		}
	}

	return ClosestEnemy;
}

void ATowerBase::TryAttackTarget()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 检查是否在攻击冷却中
	if (AbilitySystemComponent->HasMatchingGameplayTag(FValorisGameplayTags::Cooldown_Attack))
	{
		return;
	}

	// 检查当前目标是否有效
	AEnemyBase* Target = CurrentTarget.Get();
	if (Target)
	{
		// 检查目标是否在范围内
		if (AttributeSet)
		{
			const float AttackRange = AttributeSet->GetAttackRange();
			const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
			if (Distance > AttackRange)
			{
				Target = nullptr;
			}
		}

		// 检查目标是否存活
		if (Target)
		{
			if (UValorisAttributeSet* EnemyAttributes = Target->GetAttributeSet())
			{
				if (EnemyAttributes->GetHealth() <= 0.f)
				{
					Target = nullptr;
				}
			}
		}
	}

	// 如果没有有效目标，寻找新目标
	if (!Target)
	{
		Target = FindTargetEnemy();
		CurrentTarget = Target;
	}

	// 没有目标则返回
	if (!Target)
	{
		return;
	}

	// 缓存当前攻击目标，供技能使用
	CurrentTarget = Target;

	// 激活第一个技能（攻击技能）
	if (DefaultAbilities.Num() > 0 && DefaultAbilities[0])
	{
		AbilitySystemComponent->TryActivateAbilityByClass(DefaultAbilities[0]);
	}

	// Debug: 画一条线到目标
	DrawDebugLine(
		GetWorld(),
		GetActorLocation(),
		Target->GetActorLocation(),
		FColor::Red,
		false,
		0.2f,  // 持续0.2秒
		0,
		2.f    // 线宽
	);
}
