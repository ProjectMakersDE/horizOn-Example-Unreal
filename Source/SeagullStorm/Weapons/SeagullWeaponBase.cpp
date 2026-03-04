#include "Weapons/SeagullWeaponBase.h"

USeagullWeaponBase::USeagullWeaponBase()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USeagullWeaponBase::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bUnlocked) return;

	CooldownTimer -= DeltaTime;
	if (CooldownTimer <= 0.f)
	{
		Fire();
		CooldownTimer = Cooldown;
	}
}

void USeagullWeaponBase::SetStats(float InDamage, float InCooldown)
{
	Damage = InDamage;
	Cooldown = InCooldown;
}
