#include "Enemies/SeagullPoisonZone.h"
#include "Player/SeagullPlayerPawn.h"
#include "Player/SeagullHealthComponent.h"
#include "Components/SphereComponent.h"
#include "SeagullStorm.h"

ASeagullPoisonZone::ASeagullPoisonZone()
{
	PrimaryActorTick.bCanEverTick = true;

	DamageZone = CreateDefaultSubobject<USphereComponent>(TEXT("DamageZone"));
	DamageZone->SetSphereRadius(40.f);
	DamageZone->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	DamageZone->SetGenerateOverlapEvents(true);
	RootComponent = DamageZone;
}

void ASeagullPoisonZone::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(Lifetime);
}

void ASeagullPoisonZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DamageTimer -= DeltaTime;
	if (DamageTimer <= 0.f)
	{
		DamageTimer = DamageInterval;

		TArray<AActor*> Overlapping;
		DamageZone->GetOverlappingActors(Overlapping, ASeagullPlayerPawn::StaticClass());
		for (AActor* Actor : Overlapping)
		{
			ASeagullPlayerPawn* Player = Cast<ASeagullPlayerPawn>(Actor);
			if (Player && Player->HealthComponent)
			{
				Player->HealthComponent->TakeDamage(DamagePerTick);
			}
		}
	}
}
