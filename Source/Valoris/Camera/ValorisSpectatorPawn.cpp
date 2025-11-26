// Copyright Valoris. All Rights Reserved.

#include "ValorisSpectatorPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"

AValorisSpectatorPawn::AValorisSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建弹簧臂
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1500.f;
	SpringArm->SetRelativeRotation(FRotator(CameraPitch, 0.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 10.f;

	// 创建相机
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// 初始化目标距离
	TargetArmLength = SpringArm->TargetArmLength;
}

void AValorisSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

	// 设置初始俯仰角
	SpringArm->SetRelativeRotation(FRotator(CameraPitch, 0.f, 0.f));
}

void AValorisSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 屏幕边缘平移
	if (bEnableEdgePanning)
	{
		HandleEdgePanning(DeltaTime);
	}

	// 平滑缩放
	if (!FMath::IsNearlyEqual(SpringArm->TargetArmLength, TargetArmLength, 1.f))
	{
		SpringArm->TargetArmLength = FMath::FInterpTo(
			SpringArm->TargetArmLength,
			TargetArmLength,
			DeltaTime,
			10.f
		);
	}
}

void AValorisSpectatorPawn::PanCamera(const FVector2D& Direction)
{
	if (Direction.IsNearlyZero())
	{
		return;
	}

	// 获取相机的前方和右方向（忽略Z轴）
	FRotator CameraRotation = SpringArm->GetComponentRotation();
	CameraRotation.Pitch = 0.f;
	CameraRotation.Roll = 0.f;

	const FVector Forward = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);

	// 计算移动方向
	FVector MoveDirection = Forward * Direction.Y + Right * Direction.X;
	MoveDirection.Z = 0.f;
	MoveDirection.Normalize();

	// 应用移动
	FVector NewLocation = GetActorLocation() + MoveDirection * PanSpeed * GetWorld()->GetDeltaSeconds();
	SetActorLocation(NewLocation);
}

void AValorisSpectatorPawn::ZoomCamera(float Amount)
{
	if (FMath::IsNearlyZero(Amount))
	{
		return;
	}

	TargetArmLength = FMath::Clamp(
		TargetArmLength - Amount * ZoomSpeed,
		MinZoomDistance,
		MaxZoomDistance
	);
}

void AValorisSpectatorPawn::RotateCamera(float Amount)
{
	if (FMath::IsNearlyZero(Amount))
	{
		return;
	}

	FRotator CurrentRotation = SpringArm->GetRelativeRotation();
	CurrentRotation.Yaw += Amount * RotateSpeed * GetWorld()->GetDeltaSeconds();
	SpringArm->SetRelativeRotation(CurrentRotation);
}

void AValorisSpectatorPawn::FocusOnLocation(const FVector& Location)
{
	FVector NewLocation = Location;
	NewLocation.Z = GetActorLocation().Z; // 保持当前高度
	SetActorLocation(NewLocation);
}

void AValorisSpectatorPawn::HandleEdgePanning(float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	// 获取鼠标位置和视口大小
	float MouseX, MouseY;
	if (!PC->GetMousePosition(MouseX, MouseY))
	{
		return;
	}

	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

	// 计算边缘平移方向
	FVector2D PanDirection = FVector2D::ZeroVector;

	// 左边缘
	if (MouseX < EdgePanThreshold)
	{
		PanDirection.X = -1.f;
	}
	// 右边缘
	else if (MouseX > ViewportSizeX - EdgePanThreshold)
	{
		PanDirection.X = 1.f;
	}

	// 上边缘
	if (MouseY < EdgePanThreshold)
	{
		PanDirection.Y = 1.f;
	}
	// 下边缘
	else if (MouseY > ViewportSizeY - EdgePanThreshold)
	{
		PanDirection.Y = -1.f;
	}

	// 应用平移
	if (!PanDirection.IsNearlyZero())
	{
		PanCamera(PanDirection);
	}
}
