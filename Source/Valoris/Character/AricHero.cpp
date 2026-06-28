// Copyright Valoris. All Rights Reserved.

#include "AricHero.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemComponent.h"
#include "../GAS/ValorisGameplayTags.h"

AAricHero::AAricHero()
{
	// 每帧让角色面向鼠标（见 Tick）
	PrimaryActorTick.bCanEverTick = true;

	// 玩家直接 possess，不再用 HeroAIController 自动 possess
	AutoPossessAI = EAutoPossessAI::Disabled;
	AIControllerClass = nullptr;

	// 顶下相机：弹簧臂固定俯角，不随角色旋转
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 1100.f;
	CameraBoom->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f));
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bUsePawnControlRotation = false;

	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom);
	TopDownCamera->bUsePawnControlRotation = false;

	// 朝向不随控制器；M1 先朝移动方向（任务3改成朝鼠标）
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// 朝向由鼠标决定（见 Tick），不跟随移动方向
		Move->bOrientRotationToMovement = false;
	}
}

void AAricHero::BeginPlay()
{
	Super::BeginPlay();

	// 玩家与敌人不做物理阻挡：移动方扫掠的阻挡由移动者自身碰撞响应决定，
	// 敌人虽已 Ignore Pawn，但玩家走/滚向敌人时是玩家在移动，玩家自身若 Block Pawn
	// 仍会被挡停。让玩家胶囊对 Pawn 设 Ignore，玩家即可自由穿过敌人（含闪避穿透）；
	// 墙体（WorldStatic）仍正常阻挡。放 BeginPlay 而非构造函数：蓝图序列化的胶囊碰撞
	// 配置会覆盖构造函数默认，运行时设置才必定生效。
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
}

void AAricHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAricHero::OnMoveInput);
		}
		if (AttackAction)
		{
			EIC->BindAction(AttackAction, ETriggerEvent::Started, this, &AAricHero::OnAttackInput);
		}
		if (DodgeAction)
		{
			EIC->BindAction(DodgeAction, ETriggerEvent::Started, this, &AAricHero::OnDodgeInput);
		}
	}
}

void AAricHero::OnMoveInput(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Axis.IsNearlyZero())
	{
		return;
	}

	// 顶下固定视角：相机 yaw 固定为 0，世界 X=前、Y=右
	AddMovementInput(FVector::ForwardVector, Axis.Y);
	AddMovementInput(FVector::RightVector, Axis.X);
}

void AAricHero::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	// 让骑士平滑面向鼠标光标所指的地面点
	FHitResult Hit;
	if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		FVector ToCursor = Hit.Location - GetActorLocation();
		ToCursor.Z = 0.f;
		if (!ToCursor.IsNearlyZero())
		{
			const FRotator Target(0.f, ToCursor.Rotation().Yaw, 0.f);
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), Target, DeltaTime, 15.f));
		}
	}
}

void AAricHero::OnAttackInput()
{
	// 激活 Aric 普攻技能（命中判定在技能内做前方球形扫描，见 GA_MeleeAttack）
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayTagContainer AbilityTags;
		AbilityTags.AddTag(FValorisGameplayTags::Ability_Melee_Single_Aric_Attack);
		ASC->TryActivateAbilitiesByTag(AbilityTags);
	}
}

void AAricHero::OnDodgeInput()
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		FGameplayTagContainer AbilityTags;
		AbilityTags.AddTag(FValorisGameplayTags::Ability_Movement_Aric_Dodge);
		ASC->TryActivateAbilitiesByTag(AbilityTags);
	}
}
