// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardActor.generated.h"

class UWidgetComponent;
class AHexCell;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCardCellEvent, AHexCell*, Cell);

UCLASS()
class LD45_API ACardActor : public AActor
{
	GENERATED_BODY()
	
public:	
    ACardActor(const FObjectInitializer& ObjectInitializer);

public:

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool CanInteractWithCell(AHexCell* Cell) const;

    UFUNCTION(BlueprintCallable)
    void StartHoverCell(AHexCell* Cell);

    UFUNCTION(BlueprintCallable)
    void EndHoverCell(AHexCell* Cell);

    UFUNCTION(BlueprintCallable)
    bool TryInteract(AHexCell* Cell);

public:

    UPROPERTY(BlueprintAssignable)
    FCardCellEvent OnStartHoverCell;

    UPROPERTY(BlueprintAssignable)
    FCardCellEvent OnEndHoverCell;

    UPROPERTY(BlueprintAssignable)
    FCardCellEvent OnInteract;

protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UWidgetComponent* CardWidgetComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
