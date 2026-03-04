#include "Player/SeagullPlayerPawn.h"
#include "Player/SeagullHealthComponent.h"
#include "Player/SeagullXPComponent.h"
#include "Weapons/SeagullWeaponManager.h"
#include "Core/SeagullGameInstance.h"
#include "Core/SeagullStormGameMode.h"
#include "Core/SeagullPlayerController.h"
#include "Data/SeagullConfigCache.h"
#include "Components/SphereComponent.h"
#include "PaperFlipbookComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Core/SeagullTypes.h"
#include "SeagullStorm.h"

ASeagullPlayerPawn::ASeagullPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(TEXT("PlayerPawn"));

	// Collision
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetSphereRadius(16.f);
	CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = CollisionComponent;

	// Sprite
	SpriteComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(RootComponent);

	// Movement
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	MovementComponent->MaxSpeed = BaseSpeed;
	MovementComponent->Acceleration = 2000.f;
	MovementComponent->Deceleration = 2000.f;

	// Camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 500.f;
	SpringArm->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->ProjectionMode = ECameraProjectionMode::Orthographic;
	Camera->OrthoWidth = SeagullConstants::BaseResolutionWidth;

	// Components
	HealthComponent = CreateDefaultSubobject<USeagullHealthComponent>(TEXT("Health"));
	XPComponent = CreateDefaultSubobject<USeagullXPComponent>(TEXT("XP"));
	WeaponManager = CreateDefaultSubobject<USeagullWeaponManager>(TEXT("WeaponManager"));
}

void ASeagullPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	HealthComponent->OnDeath.AddDynamic(this, &ASeagullPlayerPawn::OnDeath);
	ApplyUpgrades();
}

void ASeagullPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Get movement input from controller
	ASeagullPlayerController* PC = Cast<ASeagullPlayerController>(GetController());
	if (PC)
	{
		FVector2D Input = PC->GetMoveInput();
		if (!Input.IsNearlyZero())
		{
			FVector WorldDir(Input.X, Input.Y, 0.f);
			WorldDir.Normalize();
			AddMovementInput(WorldDir, 1.0f);
			LastMoveDirection = Input.GetSafeNormal();

			// Flip sprite based on horizontal direction
			if (SpriteComponent && LastMoveDirection.X != 0.0f)
			{
				float FlipScale = (LastMoveDirection.X < 0.0f) ? -1.0f : 1.0f;
				SpriteComponent->SetRelativeScale3D(FVector(FlipScale, 1.0f, 1.0f));
			}
		}
	}

	// Clamp to arena bounds
	FVector Pos = GetActorLocation();
	float HalfW = SeagullConstants::ArenaWidth * 0.5f;
	float HalfH = SeagullConstants::ArenaHeight * 0.5f;
	Pos.X = FMath::Clamp(Pos.X, -HalfW, HalfW);
	Pos.Y = FMath::Clamp(Pos.Y, -HalfH, HalfH);
	SetActorLocation(Pos);
}

void ASeagullPlayerPawn::ApplyUpgrades()
{
	USeagullGameInstance* GI = Cast<USeagullGameInstance>(GetGameInstance());
	if (!GI || !GI->GetConfigCache()) return;

	USeagullConfigCache* Config = GI->GetConfigCache();

	// Speed
	float SpeedMult = Config->GetUpgradeValue(TEXT("speed"), GI->SaveData.GetUpgradeLevel(TEXT("speed")));
	MovementComponent->MaxSpeed = BaseSpeed * SpeedMult;

	// HP
	int32 MaxHP = static_cast<int32>(Config->GetUpgradeValue(TEXT("hp"), GI->SaveData.GetUpgradeLevel(TEXT("hp"))));
	HealthComponent->SetMaxHP(MaxHP);

	// Magnet
	float MagnetRange = Config->GetUpgradeValue(TEXT("magnet"), GI->SaveData.GetUpgradeLevel(TEXT("magnet")));
	XPComponent->PickupRadius = MagnetRange;

	// Damage multiplier applied in weapon manager
	float DamageMult = Config->GetUpgradeValue(TEXT("damage"), GI->SaveData.GetUpgradeLevel(TEXT("damage")));
	WeaponManager->DamageMultiplier = DamageMult;
	WeaponManager->UpdateDamageMultipliers();
}

void ASeagullPlayerPawn::OnDeath()
{
	UE_LOG(LogSeagullStorm, Log, TEXT("Player died!"));

	ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->EndRun(true);
	}
}
