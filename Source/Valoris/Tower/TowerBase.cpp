// Copyright Valoris. All Rights Reserved.

#include "TowerBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "../GAS/ValorisAttributeSet.h"

ATowerBase::ATowerBase()
{
	PrimaryActorTick.bCanEverTick = false;

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
