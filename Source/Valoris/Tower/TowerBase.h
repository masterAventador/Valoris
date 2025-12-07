// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "TowerBase.generated.h"

class UAbilitySystemComponent;
class UValorisAttributeSet;
class UGameplayAbility;
class UGameplayEffect;

/**
 * 防御塔基类
 * 所有塔类型（箭塔、炮塔、减速塔）都继承自此类
 */
UCLASS()
class VALORIS_API ATowerBase : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ATowerBase();

	//~ IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// 获取属性集
	UFUNCTION(BlueprintCallable, Category = "Tower")
	UValorisAttributeSet* GetAttributeSet() const { return AttributeSet; }

	// 获取建造花费
	UFUNCTION(BlueprintCallable, Category = "Tower")
	int32 GetBuildCost() const { return BuildCost; }

protected:
	virtual void BeginPlay() override;

	// 初始化 GAS
	void InitializeAbilitySystem();

	// 授予默认技能
	void GiveDefaultAbilities();

	// 应用默认属性
	void ApplyDefaultAttributes();

	//~ 组件

	// 场景根组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	TObjectPtr<USceneComponent> SceneRoot;

	// 塔的网格体（在蓝图中设置）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	TObjectPtr<UStaticMeshComponent> TowerMesh;

	// GAS 组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	// 属性集
	UPROPERTY()
	TObjectPtr<UValorisAttributeSet> AttributeSet;

	//~ 配置

	// 建造花费
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Config")
	int32 BuildCost = 100;

	// 默认技能（如自动攻击）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Config")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	// 默认属性效果
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tower|Config")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;
};
