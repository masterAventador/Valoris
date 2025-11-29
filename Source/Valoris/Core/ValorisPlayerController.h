// Copyright Valoris. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ValorisPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class AValorisSpectatorPawn;
class AValorisCharacterBase;

/**
 * 玩家控制器
 * 处理输入、相机控制、点击移动等
 */
UCLASS()
class VALORIS_API AValorisPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AValorisPlayerController();

	// 获取相机Pawn
	AValorisSpectatorPawn* GetCameraPawn() const;

	// 设置控制的英雄
	void SetControlledHero(AValorisCharacterBase* Hero);

	// 获取控制的英雄
	AValorisCharacterBase* GetControlledHero() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// 输入映射上下文
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	// 右键点击（移动/攻击）
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> RightClickAction;

	// 相机缩放
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CameraZoomAction;

	// 相机旋转
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CameraRotateAction;

	// 聚焦英雄
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> FocusHeroAction;

	// 输入处理
	void OnRightClick(const FInputActionValue& Value);
	void OnCameraZoom(const FInputActionValue& Value);
	void OnCameraRotate(const FInputActionValue& Value);
	void OnFocusHero(const FInputActionValue& Value);

private:
	// 获取鼠标点击位置的世界坐标
	bool GetMouseHitLocation(FVector& OutLocation) const;

	// 控制的英雄
	UPROPERTY()
	TObjectPtr<AValorisCharacterBase> ControlledHero;
};
