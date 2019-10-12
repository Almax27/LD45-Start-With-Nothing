// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Util.h"
#include "HexCell.generated.h"

class AHexMap;
class AMapEntity;

UCLASS(Abstract)
class LD45_API AHexCell : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AHexCell();

protected:
    
    void BeginPlay() override;
    void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:

    UFUNCTION(BlueprintCallable)
    const FHexMapCoord& GetMapCoord() const { return HexMapCoord; }

    UFUNCTION(BlueprintCallable)
    AHexMap* GetOwningMap() const;

    UFUNCTION(BlueprintCallable)
    AMapEntity* GetOccupyingEntity() const { return OccupyingEntity; }

    UFUNCTION(BlueprintCallable)
    bool GetIsTraversable(const AMapEntity* Entity = nullptr) const;

    UFUNCTION(BlueprintCallable)
    bool GetIsAttackable(const AMapEntity* Entity = nullptr) const;
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void HighlightCell(FColor Color);

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void UnhighlightCell();

    UFUNCTION(BlueprintCallable)
    bool GetIsHighlighted() const { return IsHighlighted; }
    
public:

    UFUNCTION(BlueprintImplementableEvent)
    void ShowPendingSpawn(bool IsPendingSpawn);

    UFUNCTION(BlueprintImplementableEvent)
    void ShowEnemyAttack(bool IsEnemyAttacking);

protected:

    friend class AHexMap;
    friend class AMapEntity;

    void SetMapCoord(const FHexMapCoord& Coord);
    void SetOccupyingEntity(AMapEntity* Entity);

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsTraversable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AMapEntity* OccupyingEntity;

private:

    FHexMapCoord HexMapCoord;

    bool IsHighlighted = false;

};
