// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HealthBarWidget.generated.h"

class UProgressBar;
class UAbilitySystemComponent;

/**
 * 血条 Widget
 * 可用于敌人、英雄等任何带 ASC 的角色
 */
UCLASS()
class VALORIS_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 绑定到目标角色的 ASC
	UFUNCTION(BlueprintCallable, Category = "Health")
	void BindToASC(UAbilitySystemComponent* ASC);

	// 手动更新血条（备用）
	UFUNCTION(BlueprintCallable, Category = "Health")
	void UpdateHealthBar(float HealthPercent);

protected:
	virtual void NativeConstruct() override;

	// 血量变化回调
	void OnHealthChanged(const struct FOnAttributeChangeData& Data);
	void OnMaxHealthChanged(const struct FOnAttributeChangeData& Data);

	// 更新显示
	void RefreshHealthBar();

	// 血条进度条（需要在蓝图中绑定）
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	// 缓存的血量值
	float CurrentHealth = 100.f;
	float MaxHealth = 100.f;

	// 绑定的 ASC
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> BoundASC;
};
