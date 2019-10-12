// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Util.h"
#include "VoidGameMode.generated.h"

class AMapEntity;

UENUM(BlueprintType)
enum class EGameFlowStateType : uint8
{
    None UMETA(Hidden),

    GameStart,

    ShowBoard,
    AddFirstPendingSpawns,

    GameLoopStart,
    SpawnEnemies,
    AddPendingSpawns,
    PlayerDrawCards,
    PlayerPlayCards,
    PlayerEndTurn,
    ResolveEnemyAttacks,
    EnemyTurn,
    GameLoopEnd,

    GameLost,
    GameWon,
    GameEnd
};

USTRUCT(BlueprintType)
struct FPendingEnemySpawn
{
    GENERATED_BODY()

public:

    FPendingEnemySpawn() {}
    FPendingEnemySpawn(const TSubclassOf<AMapEntity>& E, const FHexMapCoord& L) : EntityClass(E), Location(L) {}

    UPROPERTY(EditAnywhere)
    TSubclassOf<AMapEntity> EntityClass;

    UPROPERTY(EditAnywhere)
    FHexMapCoord Location;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoidGameModeEvent, AVoidGameMode*, GameMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlowStateEvent, EGameFlowStateType, State);

/**
 * 
 */
UCLASS()
class LD45_API AVoidGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

    AVoidGameMode();

protected:

    void BeginPlay() override;
	
public:

    UFUNCTION(BlueprintCallable)
    void GotoFlowState(EGameFlowStateType NewState);

    UFUNCTION(BlueprintCallable)
    void GotoNextFlowState();

    UFUNCTION(BlueprintCallable)
    EGameFlowStateType GetCurrentFlowState() const;    

    UFUNCTION(BlueprintCallable)
    TArray<AMapEntity*> GetAllEntities() const;

    UFUNCTION(BlueprintCallable)
    const TArray<AMapEntity*>& GetEnemies() const { return ActiveEnemies; }

    UFUNCTION(BlueprintCallable)
    const TArray<AMapEntity*>& GetFriendlies() const { return ActiveFriendlies; }

    UFUNCTION(BlueprintCallable)
    const TArray<FPendingEnemySpawn>& GetPendingSpawns() const { return PendingSpawns; }

    UFUNCTION(BlueprintCallable)
    void SetShowPendingSpawns(bool Show);

protected:

    UFUNCTION(BlueprintNativeEvent)
    void EnterFlowState(EGameFlowStateType FlowState);
    virtual void EnterFlowState_Implementation(EGameFlowStateType FlowState);

    UFUNCTION(BlueprintNativeEvent)
    void ExitFlowState(EGameFlowStateType FlowState);
    virtual void ExitFlowState_Implementation(EGameFlowStateType FlowState);

public:

    UFUNCTION(BlueprintCallable)
    AMapEntity* SpawnEntityOnCell(class AHexCell* Cell, TSubclassOf<AMapEntity> EntityClass);

    UFUNCTION(BlueprintCallable)
    AMapEntity* SpawnEntityAtLocation(const FHexMapCoord& Location, TSubclassOf<AMapEntity> EntityClass);

protected:

    UFUNCTION()
    void HandleEntityDestroyed(AActor* Entity);

    UFUNCTION()
    void HandleEntityDeath(AMapEntity* Entity);

    UFUNCTION()
    void DestroyAllEntities();

    TSubclassOf<AMapEntity> PickRandomEnemyType() const;

    bool FindRandomEnemySpawnLocation(FHexMapCoord& OutCoord) const;    

    UFUNCTION(BlueprintCallable)
    void AddFirstPendingSpawns();

    UFUNCTION(BlueprintCallable)
    void SpawnEnemies();

    UFUNCTION(BlueprintCallable)
    void AddPendingSpawns();

public:

    UPROPERTY(BlueprintAssignable)
    FFlowStateEvent OnEnterFlowStateEvent;

    UPROPERTY(BlueprintAssignable)
    FFlowStateEvent OnExitFlowStateEvent;

    UPROPERTY(EditDefaultsOnly)
    TArray<TSubclassOf<AMapEntity>> EnemyTypes;

protected:

    UPROPERTY(BlueprintReadWrite)
    class AHexMap* HexMapActor;

private:

    EGameFlowStateType CurrentFlowState;

    bool IsGotoStateLocked = false;
    EGameFlowStateType PendingGotoState;

    UPROPERTY(Transient)
    TArray<FPendingEnemySpawn> PendingSpawns;

    UPROPERTY(Transient)
    TArray<AMapEntity*> ActiveEnemies;

    UPROPERTY(Transient)
    TArray<AMapEntity*> ActiveFriendlies;
};
