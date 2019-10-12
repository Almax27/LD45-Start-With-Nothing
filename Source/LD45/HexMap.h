// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PaperTileMapActor.h"
#include "WeakObjectPtr.h"
#include "Util.h"
#include "HexMap.generated.h"

class USceneComponent;
class UPaperTileMap;
class AHexCell;
class UCurveFloat;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHexMapEvent, class AHexMap*, HexMap);

UENUM(BlueprintType)
enum class ECellTransitionType : uint8
{
    Left, Right, Up, Down, Random
};

USTRUCT(BlueprintType)
struct LD45_API FHexTileTypeData : public FTableRowBase
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tile)
    TSubclassOf<AHexCell> CellActorClass;
};

UCLASS(Blueprintable)
class LD45_API AHexMap : public APaperTileMapActor
{
	GENERATED_BODY()
	
public:	

	// Sets default values for this actor's properties
	AHexMap();

protected:

    virtual void OnConstruction(const FTransform& Transform) override;

    virtual void BeginPlay() override;

public:

    // Called every frame
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable)
    virtual bool SetTileMap(UPaperTileMap* NewTileMap);

    UFUNCTION(BlueprintCallable)
    const TArray<AHexCell*>& GetCells() const { return Cells; }

    UFUNCTION(BlueprintCallable)
    virtual void RefreshCells();

    UFUNCTION(BlueprintCallable)
    virtual void HighlightCells(const TArray<FHexMapCoord>& Cells, FColor Color);

    UFUNCTION(BlueprintCallable)
    virtual void UnhighlightAllCells();

    UFUNCTION(BlueprintCallable)
    const TArray<FHexMapCoord>& GetPlayerSpawnLocations() const { return PlayerSpawnLocations;}

    UFUNCTION(BlueprintCallable)
    TArray<FHexMapCoord> GetValidPlayerSpawnLocations() const;

    UFUNCTION(BlueprintCallable)
    const TArray<FHexMapCoord>& GetEnemySpawnLocations() const { return EnemySpawnLocations;}

    UFUNCTION(BlueprintCallable)
    TArray<FHexMapCoord> GetValidEnemySpawnLocations() const;

	UFUNCTION(BlueprintCallable)
    void GetAdjacentHexCoords(const FHexMapCoord& Coord, TArray<FHexMapCoord>& OutAdjacent) const;

    UFUNCTION(BlueprintCallable)
    bool GetNextCoordInDirection(const FHexMapCoord& Coord, int Direction, FHexMapCoord& OutCoord) const;

    UFUNCTION(BlueprintCallable)
    AHexCell* GetCell(int x, int y) const;    

    UFUNCTION(BlueprintCallable)
    bool IsTraversable(const FHexMapCoord& Coord, class AMapEntity* Entity = nullptr) const;

private:

    void StartTransition(bool TransitionIn);
    bool UpdateCellTransitions();

    void PostLoadCells();

public:

    UPROPERTY(BlueprintAssignable)
    FHexMapEvent MapTransitionFinishedEvent;

protected:

    UPROPERTY(EditAnywhere)
    UDataTable* TileDataTable = nullptr;

    UPROPERTY(EditAnywhere)
    TSubclassOf<class AMapEntity> BuildingEntityClass;

    UPROPERTY(EditAnywhere)
    ECellTransitionType TransitionType;

    UPROPERTY(EditAnywhere)
	UCurveFloat* TransitionCurve;

    UPROPERTY(EditAnywhere)
    float MinCellTransitionDuration;

    UPROPERTY(EditAnywhere)
    float MaxCellTransitionDuration;

private:

    UPROPERTY(Transient)
    TArray<AHexCell*> Cells;

    bool IsPendingRefresh;
    bool IsTransitioningIn;

    struct FCellTransition
    {
        TWeakObjectPtr<AHexCell> Cell;
        float Duration;
        float OriginHeight;
    };
    TArray<FCellTransition> CellTransitions;

    float CellTransitionTick = 0.0f;
    
private:

    int CellsWidth = 0;
    int CellsHeight = 0;

    TArray<FHexMapCoord> PlayerSpawnLocations;
    TArray<FHexMapCoord> EnemySpawnLocations;
    TArray<FHexMapCoord> BuildingSpawnLocations;

};
