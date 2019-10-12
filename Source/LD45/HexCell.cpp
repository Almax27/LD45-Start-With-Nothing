#include "HexCell.h"
#include "HexMap.h"
#include "MapEntity.h"

AHexCell::AHexCell()
{
    IsTraversable = true;
}

void AHexCell::BeginPlay()
{
	Super::BeginPlay();
}

void AHexCell::EndPlay(EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    if (OccupyingEntity)
    {
        OccupyingEntity->Destroy();
    }
}

AHexMap* AHexCell::GetOwningMap() const
{
    return Cast<AHexMap>(GetOwner());
}

bool AHexCell::GetIsTraversable(const AMapEntity* Entity) const
{
    return IsTraversable && OccupyingEntity == nullptr;
}

bool AHexCell::GetIsAttackable(const AMapEntity* Entity) const
{
    return IsTraversable;
}

void AHexCell::SetMapCoord(const FHexMapCoord& Coord)
{
    HexMapCoord = Coord;
}

void AHexCell::SetOccupyingEntity(AMapEntity* Entity)
{
    OccupyingEntity = Entity;
}

void AHexCell::HighlightCell_Implementation(FColor Color)
{
    IsHighlighted = true;
}

void AHexCell::UnhighlightCell_Implementation()
{
    IsHighlighted = false;
}

