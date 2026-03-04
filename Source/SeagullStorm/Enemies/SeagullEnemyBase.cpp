#include "Enemies/SeagullEnemyBase.h"
#include "Player/SeagullPlayerPawn.h"
#include "Player/SeagullHealthComponent.h"
#include "Pickups/SeagullPickup_XP.h"
#include "Core/SeagullStormGameState.h"
#include "Core/SeagullStormGameMode.h"
#include "Audio/SeagullAudioManager.h"
#include "Components/SphereComponent.h"
#include "PaperFlipbookComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SeagullStorm.h"

ASeagullEnemyBase::ASeagullEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(TEXT("Enemy"));

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	CollisionComponent->SetSphereRadius(14.f);
	CollisionComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionComponent->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComponent;

	SpriteComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Sprite"));
	SpriteComponent->SetupAttachment(RootComponent);
}

void ASeagullEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &ASeagullEnemyBase::OnOverlapBegin);
}

void ASeagullEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	MoveTowardPlayer(DeltaTime);
	AttackTimer -= DeltaTime;
}

void ASeagullEnemyBase::InitFromStats(const FSeagullEnemyStats& Stats)
{
	MaxHP = Stats.HP;
	CurrentHP = Stats.HP;
	MoveSpeed = Stats.Speed;
	ContactDamage = Stats.Damage;
	XPDrop = Stats.XPDrop;
}

void ASeagullEnemyBase::TakeDamageAmount(int32 Amount)
{
	if (IsDead()) return;

	CurrentHP -= Amount;

	// Play enemy hit SFX
	UWorld* World = GetWorld();
	if (World)
	{
		ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(World->GetAuthGameMode());
		if (GM && GM->AudioManager && GM->AudioManager->SFX_EnemyHit)
		{
			GM->AudioManager->PlaySFX(GM->AudioManager->SFX_EnemyHit, World);
		}
	}

	if (CurrentHP <= 0)
	{
		OnDeath();
	}
}

void ASeagullEnemyBase::MoveTowardPlayer(float DeltaTime)
{
	AActor* Player = GetPlayerPawn();
	if (!Player) return;

	FVector Dir = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	FVector NewLoc = GetActorLocation() + Dir * MoveSpeed * DeltaTime;
	SetActorLocation(NewLoc);

	// Flip sprite based on horizontal direction
	if (SpriteComponent)
	{
		float FlipScale = (Dir.X < 0.0f) ? -1.0f : 1.0f;
		SpriteComponent->SetRelativeScale3D(FVector(FlipScale, 1.0f, 1.0f));
	}
}

void ASeagullEnemyBase::OnDeath()
{
	// Spawn XP pickup
	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ASeagullPickup_XP* XP = World->SpawnActor<ASeagullPickup_XP>(
			ASeagullPickup_XP::StaticClass(), GetActorLocation(), FRotator::ZeroRotator, Params);
		if (XP)
		{
			XP->XPAmount = XPDrop;
		}
	}

	// Update game state
	ASeagullStormGameState* GS = World ? World->GetGameState<ASeagullStormGameState>() : nullptr;
	if (GS)
	{
		GS->AddKill();
	}

	Destroy();
}

AActor* ASeagullEnemyBase::GetPlayerPawn() const
{
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void ASeagullEnemyBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	ASeagullPlayerPawn* Player = Cast<ASeagullPlayerPawn>(OtherActor);
	if (Player && AttackTimer <= 0.f)
	{
		Player->HealthComponent->TakeDamage(ContactDamage);
		AttackTimer = AttackCooldown;

		// Play enemy attack SFX
		UWorld* World = GetWorld();
		if (World)
		{
			ASeagullStormGameMode* GM = Cast<ASeagullStormGameMode>(World->GetAuthGameMode());
			if (GM && GM->AudioManager && GM->AudioManager->SFX_EnemyAttack)
			{
				GM->AudioManager->PlaySFX(GM->AudioManager->SFX_EnemyAttack, World);
			}
		}
	}
}
