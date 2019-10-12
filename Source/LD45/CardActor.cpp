// Fill out your copyright notice in the Description page of Project Settings.


#include "CardActor.h"
#include "Components/WidgetComponent.h"

ACardActor::ACardActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CardWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("CardWidget"));
}

void ACardActor::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ACardActor::CanInteractWithCell_Implementation(AHexCell* Cell) const
{
    return false;
}

void ACardActor::StartHoverCell(AHexCell* Cell)
{
    OnStartHoverCell.Broadcast(Cell);
}

void ACardActor::EndHoverCell(AHexCell* Cell)
{
    OnEndHoverCell.Broadcast(Cell);
}

bool ACardActor::TryInteract(AHexCell* Cell)
{
    if (CanInteractWithCell(Cell))
    {
        OnInteract.Broadcast(Cell);
        return true;
    }
    return false;
}

