#pragma once
#include "CoreMinimal.h"
/** Paper2D local X/Z must both lie on arena X/Y. A pitch rotation leaves the sprite vertical. */
inline FQuat SeagullSpritePlane()
{
    return FRotationMatrix::MakeFromXZ(FVector::ForwardVector,FVector::RightVector).ToQuat();
}
