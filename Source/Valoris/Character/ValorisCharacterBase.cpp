// Copyright Valoris. All Rights Reserved.

#include "ValorisCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "../GAS/ValorisAttributeSet.h"

AValorisCharacterBase::AValorisCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	// 子类各自配置 AIController
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 创建 ASC
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// 创建属性集
	AttributeSet = CreateDefaultSubobject<UValorisAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AValorisCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AValorisCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeAbilitySystem();
}

void AValorisCharacterBase::InitializeAbilitySystem()
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

void AValorisCharacterBase::GiveDefaultAbilities()
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

void AValorisCharacterBase::ApplyDefaultAttributes()
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
