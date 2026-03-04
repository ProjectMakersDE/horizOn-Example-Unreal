#include "Weapons/SeagullWeaponManager.h"
#include "Weapons/SeagullWeapon_Feather.h"
#include "Weapons/SeagullWeapon_Screech.h"
#include "Weapons/SeagullWeapon_Dive.h"
#include "Weapons/SeagullWeapon_Gust.h"
#include "SeagullStorm.h"

USeagullWeaponManager::USeagullWeaponManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USeagullWeaponManager::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Create all weapon components (only feather starts unlocked)
	FeatherWeapon = NewObject<USeagullWeapon_Feather>(Owner);
	FeatherWeapon->RegisterComponent();
	FeatherWeapon->Unlock();

	ScreechWeapon = NewObject<USeagullWeapon_Screech>(Owner);
	ScreechWeapon->RegisterComponent();

	DiveWeapon = NewObject<USeagullWeapon_Dive>(Owner);
	DiveWeapon->RegisterComponent();

	GustWeapon = NewObject<USeagullWeapon_Gust>(Owner);
	GustWeapon->RegisterComponent();

	UpdateDamageMultipliers();
}

void USeagullWeaponManager::UnlockWeapon(ESeagullWeaponType Type)
{
	USeagullWeaponBase* Weapon = GetWeapon(Type);
	if (Weapon)
	{
		Weapon->Unlock();
	}
}

void USeagullWeaponManager::UpgradeWeapon(ESeagullWeaponType Type)
{
	USeagullWeaponBase* Weapon = GetWeapon(Type);
	if (Weapon)
	{
		Weapon->Level++;
	}
}

USeagullWeaponBase* USeagullWeaponManager::GetWeapon(ESeagullWeaponType Type) const
{
	switch (Type)
	{
		case ESeagullWeaponType::Feather: return FeatherWeapon;
		case ESeagullWeaponType::Screech: return ScreechWeapon;
		case ESeagullWeaponType::Dive: return DiveWeapon;
		case ESeagullWeaponType::Gust: return GustWeapon;
	}
	return nullptr;
}

void USeagullWeaponManager::UpdateDamageMultipliers()
{
	if (FeatherWeapon) FeatherWeapon->DamageMultiplier = DamageMultiplier;
	if (ScreechWeapon) ScreechWeapon->DamageMultiplier = DamageMultiplier;
	if (DiveWeapon) DiveWeapon->DamageMultiplier = DamageMultiplier;
	if (GustWeapon) GustWeapon->DamageMultiplier = DamageMultiplier;
}
