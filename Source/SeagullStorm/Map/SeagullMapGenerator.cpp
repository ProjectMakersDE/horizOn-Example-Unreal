#include "Map/SeagullMapGenerator.h"
#include "Core/SeagullTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
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
	// Spawn a simple static mesh plane scaled to tile size for arena visualization
	UStaticMeshComponent* Tile = NewObject<UStaticMeshComponent>(this);
	if (!Tile) return;

	Tile->SetupAttachment(RootComponent);
	Tile->RegisterComponent();
	Tile->SetWorldLocation(Position);
	Tile->SetWorldScale3D(FVector(0.16f, 0.16f, 1.f)); // Scale default 100cm plane to 16cm

	// Use engine's default plane mesh
	static UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMesh)
	{
		Tile->SetStaticMesh(PlaneMesh);
	}

	// Create and set colored material
	UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(
		Tile->GetMaterial(0), this);
	if (Mat)
	{
		Mat->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Tile->SetMaterial(0, Mat);
	}
}
