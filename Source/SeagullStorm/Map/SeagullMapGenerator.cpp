#include "Map/SeagullMapGenerator.h"
#include "Core/SeagullTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SeagullStorm.h"

ASeagullMapGenerator::ASeagullMapGenerator()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ASeagullMapGenerator::BeginPlay()
{
	Super::BeginPlay();
	GenerateArena();
}

void ASeagullMapGenerator::GenerateArena()
{
	// Generate a simple sand arena with water border
	float TileSize = 16.f;
	float ArenaW = SeagullConstants::ArenaWidth;
	float ArenaH = SeagullConstants::ArenaHeight;
	float BorderWidth = TileSize * 2;

	int32 TilesX = static_cast<int32>((ArenaW + BorderWidth * 2) / TileSize);
	int32 TilesY = static_cast<int32>((ArenaH + BorderWidth * 2) / TileSize);

	float StartX = -(ArenaW + BorderWidth * 2) * 0.5f;
	float StartY = -(ArenaH + BorderWidth * 2) * 0.5f;

	for (int32 y = 0; y < TilesY; y++)
	{
		for (int32 x = 0; x < TilesX; x++)
		{
			FVector Pos(StartX + x * TileSize + TileSize * 0.5f,
				StartY + y * TileSize + TileSize * 0.5f, -1.f);

			// Determine if border (water) or arena (sand)
			float LocalX = Pos.X + ArenaW * 0.5f;
			float LocalY = Pos.Y + ArenaH * 0.5f;

			bool bIsBorder = (LocalX < 0 || LocalX > ArenaW ||
				LocalY < 0 || LocalY > ArenaH);

			SpawnGroundTile(Pos, bIsBorder ? SeagullColors::Water : SeagullColors::Sand);
		}
	}

	UE_LOG(LogSeagullStorm, Log, TEXT("Arena generated: %dx%d tiles"), TilesX, TilesY);
}

void ASeagullMapGenerator::SpawnGroundTile(const FVector& Position, const FLinearColor& Color)
{
	// In a full implementation, these would be PaperSprite instances.
	// For the SDK example, the ground is represented by the background color.
	// The actual visual tiles would be created via Paper2D tile maps in the editor.
	// This placeholder tracks the arena bounds for gameplay logic.
}
