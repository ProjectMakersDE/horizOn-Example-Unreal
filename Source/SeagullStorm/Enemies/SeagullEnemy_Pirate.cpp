#include "Enemies/SeagullEnemy_Pirate.h"
#include "Kismet/GameplayStatics.h"

ASeagullEnemy_Pirate::ASeagullEnemy_Pirate()
{
	EnemyType = ESeagullEnemyType::Pirate;
	MoveSpeed = 100.f;
	MaxHP = 40;
	CurrentHP = 40;
	ContactDamage = 20;
	XPDrop = 25;
}

void ASeagullEnemy_Pirate::MoveTowardPlayer(float DeltaTime)
{
	AActor* Player = GetPlayerPawn();
	if (!Player) return;

	// Zigzag evasive movement
	ZigzagTimer += DeltaTime * 3.f;
	ZigzagOffset = FMath::Sin(ZigzagTimer) * 30.f;

	FVector ToPlayer = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector Perpendicular(-ToPlayer.Y, ToPlayer.X, 0.f);

	FVector Dir = ToPlayer + Perpendicular * (ZigzagOffset / 30.f) * 0.3f;
	Dir.Normalize();

	FVector NewLoc = GetActorLocation() + Dir * MoveSpeed * DeltaTime;
	SetActorLocation(NewLoc);
}
