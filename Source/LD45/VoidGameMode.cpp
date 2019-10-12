// Fill out your copyright notice in the Description page of Project Settings.


#include "VoidGameMode.h"
#include "HexMap.h"
#include "HexCell.h"
#include "MapEntity.h"
#include "PaperTileMapComponent.h"
#include "Util.h"

DEFINE_LOG_CATEGORY_STATIC(LogVoidGameMode, Log, All)

AVoidGameMode::AVoidGameMode()
{
}

void AVoidGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    CurrentFlowState = EGameFlowStateType::GameStart;

    UE_LOG(LogVoidGameMode, Log, TEXT("[FlowState] ==> %s"), *GETENUMSTRING(EGameFlowStateType, CurrentFlowState));

    EnterFlowState(CurrentFlowState);
}

void AVoidGameMode::GotoFlowState(EGameFlowStateType NewState)
{
    if (NewState == CurrentFlowState)
    {
        return; //Already in this state
    }

    if (IsGotoStateLocked)
    {
        if (PendingGotoState != EGameFlowStateType::None)
        {
            UE_LOG(LogVoidGameMode, Log, TEXT("[FlowState] SKIPPED %s"), *GETENUMSTRING(EGameFlowStateType, PendingGotoState));
        }
        PendingGotoState = NewState;
        return;
    }

    UE_LOG(LogVoidGameMode, Log, TEXT("[FlowState] ==> %s"), *GETENUMSTRING(EGameFlowStateType, NewState));

    IsGotoStateLocked = true;

    ExitFlowState(CurrentFlowState);
    OnExitFlowStateEvent.Broadcast(CurrentFlowState);

    CurrentFlowState = NewState;

    EnterFlowState(CurrentFlowState);
    OnEnterFlowStateEvent.Broadcast(CurrentFlowState);

    IsGotoStateLocked = false;

    if (PendingGotoState != EGameFlowStateType::None)
    {
        auto NextState = PendingGotoState;
        PendingGotoState = EGameFlowStateType::None;
        GotoFlowState(NextState);
    }
}

void AVoidGameMode::GotoNextFlowState()
{
    if (CurrentFlowState == EGameFlowStateType::GameLoopEnd)
    {
        GotoFlowState(EGameFlowStateType::GameLoopStart);
    }
    else if (CurrentFlowState == EGameFlowStateType::GameEnd)
    {
        GotoFlowState(EGameFlowStateType::GameStart);
    }
    else
    {
        GotoFlowState((EGameFlowStateType)((uint8)CurrentFlowState + 1));
    }
}

EGameFlowStateType AVoidGameMode::GetCurrentFlowState() const
{
    return CurrentFlowState;
}

TArray<AMapEntity*> AVoidGameMode::GetAllEntities() const
{
    TArray<AMapEntity*> AllEntities;
    AllEntities.Append(ActiveEnemies);
    AllEntities.Append(ActiveFriendlies);
    return AllEntities;
}

void AVoidGameMode::SetShowPendingSpawns(bool Show)
{
    if (HexMapActor)
    {
        for (const auto& PendingSpawn : PendingSpawns)
        {
            if (auto Cell = HexMapActor->GetCell(PendingSpawn.Location.x, PendingSpawn.Location.y))
            {
                Cell->ShowPendingSpawn(Show);
            }
        }
    }
}
void AVoidGameMode::EnterFlowState_Implementation(EGameFlowStateType FlowState)
{
}

void AVoidGameMode::ExitFlowState_Implementation(EGameFlowStateType FlowState)
{
}

AMapEntity* AVoidGameMode::SpawnEntityOnCell(class AHexCell* Cell, TSubclassOf<AMapEntity> EntityClass)
{
    if (Cell)
    {
        if(auto Occupier = Cell->GetOccupyingEntity())
        {
            Occupier->Kill();
        }
        if(Cell->GetOccupyingEntity() != nullptr)
        {
            UE_LOG(LogVoidGameMode, Log, TEXT("[Spawn] FAILED %s at %s: CELL OCUPIED"), *GetNameSafe(EntityClass), *GetNameSafe(Cell));
            return nullptr;
        }
        if (AMapEntity* Entity = GetWorld()->SpawnActor<AMapEntity>(EntityClass))
        {
            UE_LOG(LogVoidGameMode, Log, TEXT("[Spawn] %s at %s"), *GetNameSafe(EntityClass), *GetNameSafe(Cell));
            
            Entity->MoveToMapCell(Cell);
            
            Entity->OnDestroyed.AddDynamic(this, &AVoidGameMode::HandleEntityDestroyed);
            Entity->OnDeath.AddDynamic(this, &AVoidGameMode::HandleEntityDeath);

            auto& ActiveSet = Entity->GetIsFriendly() ? ActiveFriendlies : ActiveEnemies;
            ActiveSet.Emplace(Entity);

            return Entity;
        }
    }
    UE_LOG(LogVoidGameMode, Error, TEXT("[Spawn] FAILED %s at %s"), *GetNameSafe(EntityClass), *GetNameSafe(Cell));
    return nullptr;
}

AMapEntity* AVoidGameMode::SpawnEntityAtLocation(const FHexMapCoord& Location, TSubclassOf<AMapEntity> EntityClass)
{
    if (ensure(HexMapActor))
    {
        return SpawnEntityOnCell(HexMapActor->GetCell(Location.x, Location.y), EntityClass);
    }
    return nullptr;
}

void AVoidGameMode::HandleEntityDestroyed(AActor* Entity)
{
    ActiveEnemies.Remove(Cast<AMapEntity>(Entity));
    ActiveFriendlies.Remove(Cast<AMapEntity>(Entity));
}

void AVoidGameMode::HandleEntityDeath(AMapEntity* Entity)
{
    ActiveEnemies.Remove(Entity);
    ActiveFriendlies.Remove(Entity);
}

void AVoidGameMode::DestroyAllEntities()
{
    auto AllEntities = GetAllEntities();
    for (auto Entity : AllEntities)
    {
        Entity->Destroy();
    }
}

TSubclassOf<AMapEntity> AVoidGameMode::PickRandomEnemyType() const
{
    int Index = FMath::RandRange(0, EnemyTypes.Num() - 1);
    if (EnemyTypes.IsValidIndex(Index))
    {
        return EnemyTypes[Index];
    }
    return nullptr;
}

bool AVoidGameMode::FindRandomEnemySpawnLocation(FHexMapCoord& OutCoord) const
{
    if (HexMapActor)
    {
        auto ValidLocations = HexMapActor->GetValidEnemySpawnLocations();
        if (ValidLocations.Num() > 0)
        {
            OutCoord = ValidLocations[FMath::RandRange(0, ValidLocations.Num() - 1)];
            return true;
        }
    }
    return false;
}

void AVoidGameMode::AddFirstPendingSpawns()
{
    SetShowPendingSpawns(false);
    PendingSpawns.Reset();
    AddPendingSpawns();
}

void AVoidGameMode::SpawnEnemies()
{
    if (HexMapActor)
    {
        for (const auto& Spawn : PendingSpawns)
        {
            SpawnEntityAtLocation(Spawn.Location, Spawn.EntityClass);
        }
    }

    SetShowPendingSpawns(false);
    PendingSpawns.Reset();
}

void AVoidGameMode::AddPendingSpawns()
{
    auto ValidLocations = HexMapActor->GetValidEnemySpawnLocations();
    Shuffle(ValidLocations);

    int MaxEnemies = 5;
    int EnemiesToSpawn = FMath::Min(3, MaxEnemies - ActiveEnemies.Num());
    for (int i = 0; i < EnemiesToSpawn && i < ValidLocations.Num(); i++)
    {
        if (auto EnemyType = PickRandomEnemyType())
        {
            UE_LOG(LogVoidGameMode, Log, TEXT("[Spawn] PENDING %s (%d,%d)"), *GetNameSafe(EnemyType), ValidLocations[i].x, ValidLocations[i].y);
            PendingSpawns.Emplace(EnemyType, ValidLocations[i]);
        }
    }

    SetShowPendingSpawns(true);
}