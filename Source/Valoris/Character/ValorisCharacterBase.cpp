// Copyright Valoris. All Rights Reserved.

#include "ValorisCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Components/SkeletalMeshComponent.h"
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

	// 创建武器组件（暂不附加，BeginPlay 时检查插槽后附加）
	WeaponRight = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponRight"));
	WeaponLeft = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponLeft"));
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

void AValorisCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	AttachWeaponsToSockets();
}

void AValorisCharacterBase::AttachWeaponsToSockets()
{
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (!CharacterMesh)
	{
		return;
	}

	// 检查右手插槽并附加武器
	if (WeaponRight && !RightHandSocketName.IsNone())
	{
		if (CharacterMesh->DoesSocketExist(RightHandSocketName))
		{
			WeaponRight->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightHandSocketName);
		}
	}

	// 检查左手插槽并附加武器
	if (WeaponLeft && !LeftHandSocketName.IsNone())
	{
		if (CharacterMesh->DoesSocketExist(LeftHandSocketName))
		{
			WeaponLeft->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftHandSocketName);
		}
	}
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
