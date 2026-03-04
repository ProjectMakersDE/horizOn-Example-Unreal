#include "Enemies/SeagullEnemy_Crab.h"

ASeagullEnemy_Crab::ASeagullEnemy_Crab()
{
	EnemyType = ESeagullEnemyType::Crab;
	// Default stats (overridden by InitFromStats)
	MoveSpeed = 40.f;
	MaxHP = 30;
	CurrentHP = 30;
	ContactDamage = 10;
	XPDrop = 10;
}
