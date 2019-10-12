// Fill out your copyright notice in the Description page of Project Settings.


#include "HexMap.h"
#include "HexCell.h"
#include "PaperTileMap.h"
#include "PaperTileMapComponent.h"
#include "PaperTileSet.h"
#include "Engine/World.h"
#include "Curves/CurveFloat.h"
#include "MapEntity.h"
#include "Engine/World.h"
#include "VoidGameMode.h"
#include "Components/StaticMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogHexMap, Log, All)

// Sets default values
AHexMap::AHexMap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    MinCellTransitionDuration = 0.5f;
    MaxCellTransitionDuration = 1.0f;

    IsPendingRefresh = false;
    IsTransitioningIn = true;

    CellTransitionTick = 0.0f;
}

void AHexMap::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
}

void AHexMap::BeginPlay()
{
	Super::BeginPlay();
}

void AHexMap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (CellTransitions.Num() > 0)
    {
        CellTransitionTick += DeltaTime;

        if (UpdateCellTransitions()) //Returns true when finished
        {
            CellTransitions.Reset();

            if (IsPendingRefresh)
            {
                if (IsTransitioningIn)
                {
                    StartTransition(false); //Transition out first, before refreshing
                }
                else
                {
                    RefreshCells();
                }
            }
            else
            {
                MapTransitionFinishedEvent.Broadcast(this);
            }
        }
    }
}

bool AHexMap::SetTileMap(UPaperTileMap* NewTileMap)
{
    if (auto MapComponent = GetRenderComponent())
    {
        MapComponent->SetTileMap(NewTileMap);

        if (Cells.Num() == 0)
        {
            RefreshCells();
        }
        else
        {
            IsPendingRefresh = true;
            StartTransition(false);
        }
        return true;
    }
    return false;
}

void AHexMap::RefreshCells()
{
    //If we're currently transitioning then mark pending
    if (CellTransitions.Num() > 0)
    {
        IsPendingRefresh = true;
        return;
    }

    //Cleanup up existing cells
    for (const auto& Cell : Cells)
    {
        if (Cell->IsValidLowLevelFast())
        {
            Cell->Destroy();
        }
    }
    Cells.Empty();
    CellsWidth = CellsHeight = 0;

    //Generate new ones
    auto MapComponent = GetRenderComponent();
    if (MapComponent && MapComponent->TileMap && TileDataTable)
    {
        int NumLayers;
        MapComponent->GetMapSize(CellsWidth, CellsHeight, NumLayers);

        for (int y = 0; y < CellsHeight; y++)
        {
            for (int x = 0; x < CellsWidth; x++)
            {
                auto TileInfo = MapComponent->GetTile(x, y, 0);

                AHexCell* NewCell = nullptr;

                if (TileInfo.TileSet)
                {
                    auto TileMetaData = TileInfo.TileSet->GetTileMetadata(TileInfo.GetTileIndex());
                    if (ensureMsgf(TileMetaData && TileMetaData->HasMetaData(), TEXT("No meta data for index %d"), TileInfo.GetTileIndex()))
                    {
                        FName TileType = TileMetaData->UserDataName;
                        FString Context = FString::Format(TEXT("Tile_({0},{1})"), { x,y });
                        if (auto TileData = TileDataTable->FindRow<FHexTileTypeData>(TileType, Context))
                        {
                            if (ensureMsgf(TileData->CellActorClass, TEXT("No cell class set for tile: %s"), *TileType.ToString()))
                            {
                                FVector TileLocation = MapComponent->GetTileCenterPosition(x, y, 0, true);
                                FActorSpawnParameters SpawnParams;
                                SpawnParams.Owner = this;
                                SpawnParams.Name = FName(*Context);
                                NewCell = GetWorld()->SpawnActor<AHexCell>(TileData->CellActorClass, TileLocation, FRotator::ZeroRotator, SpawnParams);
                                if (NewCell)
                                {
                                    NewCell->SetMapCoord({ x,y });
                                    NewCell->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
                                }
                            }
                        }
                    }
                }

                Cells.Add(NewCell);
            }
        }
    }

    StartTransition(true);

    PostLoadCells();

    IsPendingRefresh = false;
}

void AHexMap::HighlightCells(const TArray<FHexMapCoord>& CellsToHighlight, FColor Color)
{
    for (const auto& Location : CellsToHighlight)
    {
        if (auto Cell = GetCell(Location.x, Location.y))
        {
            Cell->HighlightCell(Color);
        }
    }
}

void AHexMap::UnhighlightAllCells()
{
    for (auto Cell : Cells)
    {
        Cell->UnhighlightCell();
    }
}

TArray<FHexMapCoord> AHexMap::GetValidPlayerSpawnLocations() const
{
    TArray<FHexMapCoord> Result;
    Result.Reserve(PlayerSpawnLocations.Num());

    for (const auto& Location : PlayerSpawnLocations)
    {
        if (IsTraversable(Location))
        {
            Result.Emplace(Location);
        }
    }

    return Result;
}

TArray<FHexMapCoord> AHexMap::GetValidEnemySpawnLocations() const
{
    TArray<FHexMapCoord> Result;
    Result.Reserve(EnemySpawnLocations.Num());

    for (const auto& Location : EnemySpawnLocations)
    {
        if (IsTraversable(Location))
        {
            Result.Emplace(Location);
        }
    }

    return Result;
}

void AHexMap::GetAdjacentHexCoords(const FHexMapCoord& Coord, TArray<FHexMapCoord>& OutAdjacent) const
{
    const auto& Directions = GetDirectionsAt(Coord);
    for (const auto& Direction : Directions)
    {
        FHexMapCoord AdjacentCoord = Coord + Direction;
        if (AdjacentCoord.IsValid(CellsWidth, CellsHeight))
        {
            OutAdjacent.Emplace(AdjacentCoord);
        }
    }
}

bool AHexMap::GetNextCoordInDirection(const FHexMapCoord& Coord, int Direction, FHexMapCoord& OutCoord) const
{
    const auto& Directions = GetDirectionsAt(Coord);
    if(Direction >= 0 && Direction < Directions.Num())
    {
        FHexMapCoord NextCoord = Coord + Directions[Direction];
        if (NextCoord.IsValid(CellsWidth, CellsHeight))
        {
            OutCoord = NextCoord;
            return true;
        }
    }
    return false;
}

AHexCell* AHexMap::GetCell(int x, int y) const
{
    if (x >= 0 && x < CellsWidth && y >= 0 && y < CellsHeight)
    {
        int Index = x + (y * CellsWidth);
        return ensure(Cells.IsValidIndex(Index)) ? Cells[Index] : nullptr;
    }
    return nullptr;
}

bool AHexMap::IsTraversable(const FHexMapCoord& Coord, class AMapEntity* Entity) const
{
    if (AHexCell* Cell = GetCell(Coord.x, Coord.y))
    {
        if (Cell->GetIsTraversable())
        {
            return true;
        }
    }
    return false;
}

void AHexMap::StartTransition(bool TransitionIn)
{
    if (CellTransitions.Num() > 0)
    {
        UE_LOG(LogHexMap, Warning, TEXT("Failed to start transition, already running!"));
        return;
    }

    IsTransitioningIn = TransitionIn;

    //TODO: Support other transition types....
    //this->TransitionType

    for (auto Cell : Cells)
    {
        if (Cell)
        {
            FCellTransition Transition;
            Transition.Cell = Cell;
            Transition.Duration = FMath::RandRange(MinCellTransitionDuration, MaxCellTransitionDuration);
            Transition.OriginHeight = Cell->GetActorLocation().Z;
            CellTransitions.Add(Transition);
        }
    }

    CellTransitionTick = 0.0f;
    UpdateCellTransitions();
}

bool AHexMap::UpdateCellTransitions()
{
    bool IsAnimating = false;
    for (const auto& Transition : CellTransitions)
    {
        if (auto Cell = Transition.Cell.Get())
        {
            float tval = FMath::Clamp(CellTransitionTick / Transition.Duration, 0.0f, 1.0f);
            tval = IsTransitioningIn ? tval : 1.0f - tval;

            float HeightOffset = tval * -200.0f;
            if (TransitionCurve)
            {
                HeightOffset = TransitionCurve->GetFloatValue(tval);
            }
            auto Location = Cell->GetActorLocation();
            Location.Z = Transition.OriginHeight + HeightOffset;
            Cell->SetActorLocation(Location);
            IsAnimating |= CellTransitionTick < Transition.Duration;
        }
    }
    return !IsAnimating;
}

void AHexMap::PostLoadCells()
{
    //TODO: read from layers
    EnemySpawnLocations.Reset();
    if (auto TiledMap = GetRenderComponent())
    {
        int Width, Height, NumLayers;
        TiledMap->GetMapSize(Width, Height, NumLayers);

        for (int ox = 0; ox < Width && ox < 4; ox++)
        {
            for (int oy = 0; oy < Height; oy++)
            {
                EnemySpawnLocations.Emplace(ox, oy);
            }
        }
    }

    //TODO: read from layers
    PlayerSpawnLocations.Reset();
    if (auto TiledMap = GetRenderComponent())
    {
        int Width, Height, NumLayers;
        TiledMap->GetMapSize(Width, Height, NumLayers);

        for (int ox = Width - 4; ox < Width; ox++)
        {
            for (int oy = 0; oy < Height; oy++)
            {
                PlayerSpawnLocations.Emplace(ox, oy);
            }
        }
    }

    //TODO: read from layers
    BuildingSpawnLocations.Reset();
    if (auto TiledMap = GetRenderComponent())
    {
        int Width, Height, NumLayers;
        TiledMap->GetMapSize(Width, Height, NumLayers);

        for (int ox = Width - 5; ox < Width; ox++)
        {
            for (int oy = 0; oy < Height; oy++)
            {
                BuildingSpawnLocations.Emplace(ox, oy);
            }
        }
    }

    //Spawn Buildings
    if (AVoidGameMode* VoidGameMode = Cast<AVoidGameMode>(GetWorld()->GetAuthGameMode()))
    {
        Shuffle(BuildingSpawnLocations);
        for (int i = 0; i < 3 && i < BuildingSpawnLocations.Num(); i++)
        {
            VoidGameMode->SpawnEntityAtLocation(BuildingSpawnLocations[i], BuildingEntityClass);
        }
    }
}