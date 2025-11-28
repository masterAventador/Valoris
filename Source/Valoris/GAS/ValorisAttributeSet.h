// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "ValorisAttributeSet.generated.h"

/**
 * 属性集
 * 定义角色的所有属性：生命值、法力值、攻击力等
 */
UCLASS()
class VALORIS_API UValorisAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UValorisAttributeSet();

	// 属性变化前回调（用于 Clamp）
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// 属性变化后回调（用于处理伤害、死亡等）
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	// ========== 生命值 ==========
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UValorisAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Health")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UValorisAttributeSet, MaxHealth)

	// ========== 法力值 ==========
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mana")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS_BASIC(UValorisAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Mana")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS_BASIC(UValorisAttributeSet, MaxMana)

	// ========== 战斗属性 ==========
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS_BASIC(UValorisAttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS_BASIC(UValorisAttributeSet, Defense)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UValorisAttributeSet, AttackSpeed)

	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Combat")
	FGameplayAttributeData AttackRange;
	ATTRIBUTE_ACCESSORS_BASIC(UValorisAttributeSet, AttackRange)

	// ========== 移动属性 ==========
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Movement")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UValorisAttributeSet, MoveSpeed)

	// ========== Meta 属性（不持久化，用于伤害计算）==========
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UValorisAttributeSet, IncomingDamage)
};
