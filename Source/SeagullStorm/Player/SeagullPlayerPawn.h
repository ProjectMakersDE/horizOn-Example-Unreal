#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SeagullPlayerPawn.generated.h"

class UPaperFlipbookComponent;
class UFloatingPawnMovement;
class USphereComponent;
class UCameraComponent;
class USpringArmComponent;
class USeagullHealthComponent;
class USeagullXPComponent;
class USeagullWeaponManager;

UCLASS()
class ASeagullPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	ASeagullPlayerPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere)
	UPaperFlipbookComponent* SpriteComponent;

	UPROPERTY(VisibleAnywhere)
	UFloatingPawnMovement* MovementComponent;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	USeagullHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere)
	USeagullXPComponent* XPComponent;

	UPROPERTY(VisibleAnywhere)
	USeagullWeaponManager* WeaponManager;

	float BaseSpeed = 200.f;
	FVector2D LastMoveDirection = FVector2D(1.f, 0.f);

	void ApplyUpgrades();

	UFUNCTION()
	void OnDeath();
};
