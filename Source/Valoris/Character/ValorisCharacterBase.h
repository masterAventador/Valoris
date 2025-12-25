// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "ValorisCharacterBase.generated.h"

class UAbilitySystemComponent;
class UValorisAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class USkeletalMeshComponent;

/**
 * 角色基类
 * 所有带 GAS 的角色（英雄、敌人、塔）都继承自此类
 */
UCLASS()
class VALORIS_API AValorisCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AValorisCharacterBase();

	// IAbilitySystemInterface 接口
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 获取属性集
	UValorisAttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// GAS 组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// 属性集
	UPROPERTY()
	TObjectPtr<UValorisAttributeSet> AttributeSet;

	// 初始化属性的 GameplayEffect
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;

	// 默认技能列表
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	// 武器组件 - 右手
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponRight;

	// 武器组件 - 左手
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponLeft;

	// 右手武器插槽名称
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName RightHandSocketName = FName("weapon_r");

	// 左手武器插槽名称
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName LeftHandSocketName = FName("weapon_l");

	// 初始化 GAS
	virtual void InitializeAbilitySystem();

	// 初始化武器附加
	void AttachWeaponsToSockets();

	// 授予默认技能
	void GiveDefaultAbilities();

	// 应用初始属性
	void ApplyDefaultAttributes();
};
