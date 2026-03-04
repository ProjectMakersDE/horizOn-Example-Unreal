#include "Enemies/SeagullEnemy_Boss.h"
#include "Core/SeagullStormGameMode.h"
#include "SeagullStorm.h"

ASeagullEnemy_Boss::ASeagullEnemy_Boss()
{
	EnemyType = ESeagullEnemyType::Boss;
	MoveSpeed = 30.f;
	MaxHP = 500;
	CurrentHP = 500;
	ContactDamage = 30;
	XPDrop = 100;

	// Larger collision for 64x64 boss
	if (CollisionComponent)
	{
		CollisionComponent->SetSphereRadius(28.f);
	}
}

void ASeagullEnemy_Boss::OnDeath()
{
	UE_LOG(LogSeagullStorm, Log, TEXT("Boss defeated!"));

	// Boss death ends the run successfully
	ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->EndRun(false);
	}

	Super::OnDeath();
}
