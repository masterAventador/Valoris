// Copyright Valoris. All Rights Reserved.

#include "ValorisPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "../Camera/ValorisSpectatorPawn.h"

AValorisPlayerController::AValorisPlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void AValorisPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 添加输入映射上下文
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// 限制鼠标在视口内（仅打包后生效）
#if !WITH_EDITOR
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
#endif
}

void AValorisPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 右键点击
		if (RightClickAction)
		{
			EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Started, this, &AValorisPlayerController::OnRightClick);
		}

		// 相机缩放
		if (CameraZoomAction)
		{
			EnhancedInputComponent->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &AValorisPlayerController::OnCameraZoom);
		}

		// 相机旋转
		if (CameraRotateAction)
		{
			EnhancedInputComponent->BindAction(CameraRotateAction, ETriggerEvent::Triggered, this, &AValorisPlayerController::OnCameraRotate);
		}

		// 聚焦英雄
		if (FocusHeroAction)
		{
			EnhancedInputComponent->BindAction(FocusHeroAction, ETriggerEvent::Started, this, &AValorisPlayerController::OnFocusHero);
		}
	}
}

AValorisSpectatorPawn* AValorisPlayerController::GetCameraPawn() const
{
	return Cast<AValorisSpectatorPawn>(GetPawn());
}

void AValorisPlayerController::OnRightClick(const FInputActionValue& Value)
{
	FVector HitLocation;
	if (GetMouseHitLocation(HitLocation))
	{
		// TODO: 通知英雄移动到目标位置
		UE_LOG(LogTemp, Log, TEXT("Right click at: %s"), *HitLocation.ToString());
	}
}

void AValorisPlayerController::OnCameraZoom(const FInputActionValue& Value)
{
	if (AValorisSpectatorPawn* CameraPawn = GetCameraPawn())
	{
		const float ZoomValue = Value.Get<float>();
		CameraPawn->ZoomCamera(ZoomValue);
	}
}

void AValorisPlayerController::OnCameraRotate(const FInputActionValue& Value)
{
	if (AValorisSpectatorPawn* CameraPawn = GetCameraPawn())
	{
		const float RotateValue = Value.Get<float>();
		CameraPawn->RotateCamera(RotateValue);
	}
}

void AValorisPlayerController::OnFocusHero(const FInputActionValue& Value)
{
	if (AValorisSpectatorPawn* CameraPawn = GetCameraPawn())
	{
		if (ControlledHero.IsValid())
		{
			CameraPawn->FocusOnLocation(ControlledHero->GetActorLocation());
		}
	}
}

bool AValorisPlayerController::GetMouseHitLocation(FVector& OutLocation) const
{
	FHitResult HitResult;
	if (GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		OutLocation = HitResult.Location;
		return true;
	}
	return false;
}
