#include "Player/SeagullHealthComponent.h"
#include "Core/SeagullTypes.h"

USeagullHealthComponent::USeagullHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USeagullHealthComponent::SetMaxHP(int32 NewMax)
{
	MaxHP = NewMax;
	CurrentHP = NewMax;
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);
}

void USeagullHealthComponent::TakeDamage(int32 Amount)
{
	if (IsDead() || IsInvincible()) return;

	CurrentHP = FMath::Max(0, CurrentHP - Amount);
	InvincibilityTimer = SeagullConstants::InvincibilityDuration;
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);

	if (CurrentHP <= 0)
	{
		OnDeath.Broadcast();
	}
}

void USeagullHealthComponent::Heal(int32 Amount)
{
	CurrentHP = FMath::Min(MaxHP, CurrentHP + Amount);
	OnHealthChanged.Broadcast(CurrentHP, MaxHP);
}

void USeagullHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (InvincibilityTimer > 0.f)
	{
		InvincibilityTimer -= DeltaTime;
	}
}
