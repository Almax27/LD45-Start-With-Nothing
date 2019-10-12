#include "MapEntity.h"
#include "HexMap.h"
#include "HexCell.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "VoidGameMode.h"

AMapEntity::AMapEntity()
{
    IsFriendly = false;
    MoveDistance = 3;
    AttackDistance = 1;
    MaxHealth = 1;
}

void AMapEntity::BeginPlay()
{
    Health = MaxHealth;

	Super::BeginPlay();
}

void AMapEntity::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    if (MapCell && MapCell->GetOwningMap())
    {
        MapCell->SetOccupyingEntity(nullptr);
        for (const auto& Coord : PendingAttackInfo.Locations)
        {
            if (auto Cell = MapCell->GetOwningMap()->GetCell(Coord.x, Coord.y))
            {
                Cell->ShowEnemyAttack(false);
            }
        }
    }
}

bool AMapEntity::MoveToMapCell(AHexCell* Cell)
{
    if (Cell && Cell->GetIsTraversable(this))
    {
        if (MapCell)
        {
            MapCell->SetOccupyingEntity(nullptr);
        }
        auto PreviousCell = MapCell;
        MapCell = Cell;
        MapCell->SetOccupyingEntity(this);
        this->AttachToActor(MapCell, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        OnMove.Broadcast(this, PreviousCell, Cell);
        return true;
    }
    return false;
}

bool AMapEntity::PerformAttack(const FMapAttackInfo& AttackInfo)
{
    if (ensure(MapCell) && MapCell->GetOwningMap() && AttackInfo.Locations.Num() > 0)
    {
        for (const auto& Coord : AttackInfo.Locations)
        {
            if (auto Cell = MapCell->GetOwningMap()->GetCell(Coord.x, Coord.y))
            {
                if (auto OtherEntity = Cell->GetOccupyingEntity())
                {
                    FDamageEvent DamageEvent;
                    OtherEntity->TakeDamage(AttackInfo.Damage, DamageEvent, nullptr, AttackInfo.Source);
                    OnAttack.Broadcast(this, AttackInfo);
                }
            }
        }
        return true;
    }
    return false;
}

bool AMapEntity::TryPerformAttack(AHexCell* Cell, const TArray<FMapAttackInfo>& Attacks)
{
    if (Cell)
    {
        for (const auto& Attack : Attacks)
        {
            for (const auto& Coord : Attack.Locations)
            {
                if (Coord == Cell->GetMapCoord())
                {
                    return PerformAttack(Attack);
                }
            }
        }
    }
    return false;
}

float AMapEntity::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
    DamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    int iDamage = FMath::RoundToInt(DamageAmount);
    int PreDamageHealth = Health;
    SetHealth(Health - iDamage);

    return float(PreDamageHealth - Health);
}

bool AMapEntity::Kill()
{
    if(Health > 0)
    {
        SetHealth(0);
        return true;
    }
    return false;
}

TArray<FHexMapCoord> AMapEntity::GetMoveLocations() const
{
    TArray<FHexMapCoord> Result;
    if (MapCell)
    {
        if (auto Map = MapCell->GetOwningMap())
        {
            Result.Reserve(1 + ((MoveDistance - 1) * (MoveDistance * 6)));
            TArray<FHexMapCoord> WorkingSet = { MapCell->GetMapCoord() };
            TArray<FHexMapCoord> AdjacentSet;

            for (int i = 0; i < MoveDistance; i++)
            {
                AdjacentSet.Reset();
                for (const auto& Coord : WorkingSet)
                {
                    Map->GetAdjacentHexCoords(Coord, AdjacentSet);
                }
                for (const auto& AdjacentCoord : AdjacentSet)
                {
                    WorkingSet.AddUnique(AdjacentCoord);
                }
            }

            for (const auto& Coord : WorkingSet)
            {
                auto Cell = Map->GetCell(Coord.x, Coord.y);
                if (Cell && Cell->GetIsTraversable(this))
                {
                    Result.Emplace(Coord);
                }
            }            
        }
    }
    return Result;
}

TArray<FMapAttackInfo> AMapEntity::GetAttacks() const
{
    TArray<FMapAttackInfo> Result;

    if (MapCell && AttackDistance > 0)
    {
        if (auto Map = MapCell->GetOwningMap())
        {
            Result.Reserve(AttackDistance * 6);
            const auto& SourceCoord = MapCell->GetMapCoord();

            const auto& Directions = GetDirectionsAt(SourceCoord);

            FMapAttackInfo AttackInfo;
            AttackInfo.Damage = 1;
            AttackInfo.Source = const_cast<AMapEntity*>(this);
            AttackInfo.Locations.Reserve(AttackDistance);

            for (int Dir = 0; Dir < Directions.Num(); Dir++)
            {
                AttackInfo.Locations.Reset();
                AttackInfo.Hits.Reset();

                FHexMapCoord AttackCoord = SourceCoord;
                for (int i = 0; i < AttackDistance; i++)
                {
                    if (Map->GetNextCoordInDirection(AttackCoord, Dir, AttackCoord))
                    {
                        if (auto Cell = Map->GetCell(AttackCoord.x, AttackCoord.y))
                        {
                            if (Cell->GetIsAttackable(this))
                            {
                                AttackInfo.Locations.Add(AttackCoord);

                                //Terminate attack if we hit something
                                if (auto HitEntity = Cell->GetOccupyingEntity())
                                {
                                    AttackInfo.Hits.Add(HitEntity);
                                    break;
                                }
                            }
                            else //Terminate if not attackable
                            {
                                break;
                            }
                        }
                    }
                    else //terminate if out of map bounds
                    {
                        break;
                    }
                }

                if (AttackInfo.Locations.Num() > 0)
                {
                    Result.Emplace(AttackInfo);
                }
            }
        }
    }

    return Result;
}

bool AMapEntity::AIMove()
{
    if (MapCell == nullptr) return false;

    auto Map = MapCell->GetOwningMap();
    if (Map == nullptr) return false;

    TArray<FHexMapCoord> Path;

    struct FPotentialMove
    {
        AHexCell* Cell;
        float Weight;
    };

    TArray<FPotentialMove> WorkingSet;

    auto MoveLocations = GetMoveLocations();
    for (int i = 0; i < MoveLocations.Num(); i++)
    {
        if (auto Cell = Map->GetCell(MoveLocations[i].x, MoveLocations[i].y))
        {
            float Weight = 0.0f;

            //Distance to targets
            float MinDistanceToTargetSq = FLT_MAX;
            if (auto VoidGameMode = Cast<AVoidGameMode>(GetWorld()->GetAuthGameMode()))
            {
                for (auto Friendly : VoidGameMode->GetFriendlies())
                {
                    float DistSq = FVector::Dist2D(Cell->GetActorLocation(), Friendly->GetActorLocation());
                    if (Friendly->GetIsHighValue())
                    {
                        DistSq *= 0.5f;
                    }
                    if (DistSq < MinDistanceToTargetSq)
                    {
                        MinDistanceToTargetSq = DistSq;
                    }
                }
            }

            //TODO: Consider attacks?
            Weight = (int)MinDistanceToTargetSq;

            WorkingSet.Add({ Cell, Weight });

#if UE_BUILD_DEVELOPMENT
            UKismetSystemLibrary::DrawDebugString(this, Cell->GetActorLocation(), FString::Format(TEXT("{0}"), { Weight }), nullptr, FLinearColor::Red, 1.0f);
#endif
        }
    }

    WorkingSet.Sort([](const FPotentialMove& A, const FPotentialMove& B)
    {
        return A.Weight < B.Weight;
    });

    if (WorkingSet.Num() > 0)
    {
        FPotentialMove Move = WorkingSet[FMath::RandRange(0, FMath::Min(WorkingSet.Num() - 1, 2))];
        MoveToMapCell(Move.Cell);
        return true;
    }

    return false;
}

bool AMapEntity::AITelegraphAttack()
{
    AIHasAttackPending = false;

    if (MapCell == nullptr) return false;

    auto Map = MapCell->GetOwningMap();
    if (Map == nullptr) return false;

    //Find attacks that hit
    auto Attacks = GetAttacks();
    TArray<FMapAttackInfo> AttacksThatHit;
    for(const auto& Attack : Attacks)
    {
        for(auto HitEntity : Attack.Hits)
        {
            if (HitEntity && HitEntity->GetIsFriendly())
            {
                AttacksThatHit.Add(Attack);
                break;
            }
        }
    }

    if (AttacksThatHit.Num() > 0)
    {
        AIHasAttackPending = true;
        PendingAttackInfo = AttacksThatHit[FMath::RandRange(0, AttacksThatHit.Num() - 1)];

        for (const auto& Coord : PendingAttackInfo.Locations)
        {
            if (auto Cell = Map->GetCell(Coord.x, Coord.y))
            {
                Cell->ShowEnemyAttack(true);
            }
        }

        return true;
    }

    return false;
}

bool AMapEntity::AIResolveAttack()
{
    if (AIHasAttackPending)
    {
        if (MapCell == nullptr) return false;

        auto Map = MapCell->GetOwningMap();
        if (Map == nullptr) return false;

        AIHasAttackPending = false;

        for (const auto& Coord : PendingAttackInfo.Locations)
        {
            if (auto Cell = Map->GetCell(Coord.x, Coord.y))
            {
                Cell->ShowEnemyAttack(false);
            }
        }

        PerformAttack(PendingAttackInfo);

        return true;
    }
    return false;
}

void AMapEntity::SetHealth(int NewHealth)
{
    NewHealth = FMath::Clamp(NewHealth, 0, MaxHealth);
    if (NewHealth != Health)
    {
        Health = NewHealth;
        HealthChanged();
        OnHealthChanged.Broadcast(this);

        if (Health == 0)
        {
            Death();
            OnDeath.Broadcast(this);
        }
    }
}

void AMapEntity::HealthChanged_Implementation()
{

}

void AMapEntity::Death_Implementation()
{

}


