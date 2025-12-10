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
class ATowerBase;
class ABuildPreview;
class UValorisHUD;

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

	//~ 建造系统

	// 进入建造模式
	UFUNCTION(BlueprintCallable, Category = "Build")
	void EnterBuildMode(TSubclassOf<ATowerBase> TowerClass);

	// 退出建造模式
	UFUNCTION(BlueprintCallable, Category = "Build")
	void ExitBuildMode();

	// 是否在建造模式
	UFUNCTION(BlueprintCallable, Category = "Build")
	bool IsInBuildMode() const { return bInBuildMode; }

	// 确认建造
	UFUNCTION(BlueprintCallable, Category = "Build")
	void ConfirmBuild();

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

	// 左键点击（建造确认）
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LeftClickAction;

	// 取消键（ESC）
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> CancelAction;

	// 建造模式键（B）
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> BuildModeAction;

	// 建造预览类
	UPROPERTY(EditDefaultsOnly, Category = "Build")
	TSubclassOf<ABuildPreview> BuildPreviewClass;

	// 默认建造的塔类型（测试用）
	UPROPERTY(EditDefaultsOnly, Category = "Build")
	TSubclassOf<ATowerBase> DefaultTowerClass;

	//~ UI

	// HUD Widget 类
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UValorisHUD> HUDWidgetClass;

	virtual void Tick(float DeltaTime) override;

	// 输入处理
	void OnRightClick(const FInputActionValue& Value);
	void OnCameraZoom(const FInputActionValue& Value);
	void OnCameraRotate(const FInputActionValue& Value);
	void OnFocusHero(const FInputActionValue& Value);
	void OnLeftClick(const FInputActionValue& Value);
	void OnCancel(const FInputActionValue& Value);
	void OnBuildMode(const FInputActionValue& Value);

	// 更新建造预览位置
	void UpdateBuildPreview();

private:
	// 获取鼠标点击位置的世界坐标
	bool GetMouseHitLocation(FVector& OutLocation) const;

	// 获取鼠标下的 Actor
	AActor* GetActorUnderCursor() const;

	// 控制的英雄
	UPROPERTY()
	TObjectPtr<AValorisCharacterBase> ControlledHero;

	//~ 建造状态

	// 是否在建造模式
	bool bInBuildMode = false;

	// 当前选择的塔类型
	UPROPERTY()
	TSubclassOf<ATowerBase> CurrentTowerClass;

	// 建造预览 Actor
	UPROPERTY()
	TObjectPtr<ABuildPreview> BuildPreviewActor;

	//~ UI 状态

	// HUD Widget 实例
	UPROPERTY()
	TObjectPtr<UValorisHUD> HUDWidget;
};
