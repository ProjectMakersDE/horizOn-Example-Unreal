#include "Enemies/SeagullEnemy_Jellyfish.h"
#include "SeagullStorm.h"

ASeagullEnemy_Jellyfish::ASeagullEnemy_Jellyfish()
{
	EnemyType = ESeagullEnemyType::Jellyfish;
	MoveSpeed = 60.f;
	MaxHP = 50;
	CurrentHP = 50;
	ContactDamage = 15;
	XPDrop = 20;
}

void ASeagullEnemy_Jellyfish::OnDeath()
{
	// Could spawn poison zone here in future
	// For now, just use base death behavior
	Super::OnDeath();
}
