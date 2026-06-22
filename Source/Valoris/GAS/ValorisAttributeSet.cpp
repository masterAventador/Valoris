// Copyright Valoris. All Rights Reserved.

#include "ValorisAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"
#include "../Enemy/EnemyBase.h"
#include "../Character/AricHero.h"
#include "../Core/ValorisGameMode.h"
#include "Kismet/GameplayStatics.h"

UValorisAttributeSet::UValorisAttributeSet()
{
	// 默认值
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitMana(50.f);
	InitMaxMana(50.f);
	InitAttackPower(10.f);
	InitDefense(5.f);
	InitAttackSpeed(1.f);
	InitAttackRange(200.f);
	InitMoveSpeed(600.f);
	InitIncomingDamage(0.f);
}

void UValorisAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Clamp 生命值
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	// Clamp 法力值
	else if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void UValorisAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 处理伤害
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float Damage = GetIncomingDamage();
		SetIncomingDamage(0.f);

		if (Damage > 0.f)
		{
			// 伤害已在 ExecCalc_Damage 减过防御，这里不再二次减（修复双重扣防御 bug）
			const float ActualDamage = Damage;

			// 扣血
			const float NewHealth = GetHealth() - ActualDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

			// 如果血量为0，触发死亡
			if (GetHealth() <= 0.f)
			{
				// 获取角色 Actor
				AActor* OwnerActor = nullptr;
				if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
				{
					OwnerActor = ASC->GetAvatarActor();
				}

				// 如果是敌人，标记为被击杀并销毁；如果是玩家，通知 GameMode 失败
				if (AEnemyBase* Enemy = Cast<AEnemyBase>(OwnerActor))
				{
					Enemy->MarkAsKilled();
					Enemy->Destroy();
				}
				else if (Cast<AAricHero>(OwnerActor))
				{
					if (AValorisGameMode* GM = Cast<AValorisGameMode>(UGameplayStatics::GetGameMode(OwnerActor)))
					{
						GM->NotifyPlayerDied();
					}
				}
			}
		}
	}
}
