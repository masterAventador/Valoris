// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ValorisCharacterBase.h"
#include "AricHero.generated.h"

/**
 * 英雄：Aric（战士）
 * 近战物理输出，高生命值，技能带冲锋和AOE
 */
UCLASS()
class VALORIS_API AAricHero : public AValorisCharacterBase
{
	GENERATED_BODY()

public:
	AAricHero();

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// WASD 移动输入
	void OnMoveInput(const struct FInputActionValue& Value);

	// 左键攻击输入（激活 Aric 普攻技能）
	void OnAttackInput();

	// 空格闪避输入（激活闪避技能）
	void OnDodgeInput();

	// 顶下相机弹簧臂
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> CameraBoom;

	// 顶下相机
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> TopDownCamera;

	// 移动 InputAction（编辑器里指向 IA_Move）
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputAction> MoveAction;

	// 攻击 InputAction（编辑器里指向左键 IA_Attack）
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputAction> AttackAction;

	// 闪避 InputAction（编辑器里指向空格 IA_Dodge）
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<class UInputAction> DodgeAction;
};
