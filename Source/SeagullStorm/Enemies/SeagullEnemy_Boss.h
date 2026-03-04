#pragma once

#include "CoreMinimal.h"
#include "Enemies/SeagullEnemyBase.h"
#include "SeagullEnemy_Boss.generated.h"

UCLASS()
class ASeagullEnemy_Boss : public ASeagullEnemyBase
{
	GENERATED_BODY()

public:
	ASeagullEnemy_Boss();

protected:
	virtual void OnDeath() override;
};
