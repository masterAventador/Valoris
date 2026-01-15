// Copyright Valoris. All Rights Reserved.

#include "HealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "../GAS/ValorisAttributeSet.h"

void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (HealthBar)
	{
		HealthBar->SetPercent(1.0f);
	}
}

void UHealthBarWidget::BindToASC(UAbilitySystemComponent* ASC)
{
	if (!ASC)
	{
		return;
	}

	BoundASC = ASC;

	// 获取初始值
	if (const UValorisAttributeSet* Attributes = ASC->GetSet<UValorisAttributeSet>())
	{
		CurrentHealth = Attributes->GetHealth();
		MaxHealth = Attributes->GetMaxHealth();
		RefreshHealthBar();
	}

	// 监听 Health 变化
	ASC->GetGameplayAttributeValueChangeDelegate(
		UValorisAttributeSet::GetHealthAttribute()
	).AddUObject(this, &UHealthBarWidget::OnHealthChanged);

	// 监听 MaxHealth 变化
	ASC->GetGameplayAttributeValueChangeDelegate(
		UValorisAttributeSet::GetMaxHealthAttribute()
	).AddUObject(this, &UHealthBarWidget::OnMaxHealthChanged);
}

void UHealthBarWidget::UpdateHealthBar(float HealthPercent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(FMath::Clamp(HealthPercent, 0.f, 1.f));
	}
}

void UHealthBarWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	CurrentHealth = Data.NewValue;
	RefreshHealthBar();
}

void UHealthBarWidget::OnMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	MaxHealth = Data.NewValue;
	RefreshHealthBar();
}

void UHealthBarWidget::RefreshHealthBar()
{
	if (HealthBar && MaxHealth > 0.f)
	{
		float Percent = CurrentHealth / MaxHealth;
		HealthBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
}
