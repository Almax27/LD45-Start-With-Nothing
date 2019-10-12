// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardWidget.generated.h"

/**
 * 
 */
UCLASS()
class LD45_API UCardWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:

    UFUNCTION(BlueprintNativeEvent)
    void Drawn();

    UFUNCTION(BlueprintNativeEvent)
    void Discarded();

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int Cost = 1;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool IsUnitCard = false;
};
