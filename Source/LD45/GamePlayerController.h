// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GamePlayerController.generated.h"

class UCardWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVoidPlayerEvent, AGamePlayerController*, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVoidPlayerCardEvent, AGamePlayerController*, Player, UCardWidget*, Card);

/**
 * 
 */
UCLASS()
class LD45_API AGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

    void BeginPlay() override;

public:

    UFUNCTION(BlueprintCallable)
    void ResetCards();

    UFUNCTION(BlueprintCallable)
    UCardWidget* DrawCard();

    UFUNCTION(BlueprintCallable)
    bool DiscardCard(UCardWidget* Card);

    UFUNCTION(BlueprintCallable)
    void DiscardAll();

    UFUNCTION(BlueprintCallable)
    void StartShuffle();

    UFUNCTION(BlueprintCallable)
    void EndShuffle();

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<TSubclassOf<UCardWidget>> Deck;

    UPROPERTY(BlueprintAssignable)
    FVoidPlayerCardEvent OnCardDrawn;

    UPROPERTY(BlueprintAssignable)
    FVoidPlayerCardEvent OnCardDiscarded;

    UPROPERTY(BlueprintAssignable)
    FVoidPlayerEvent OnShuffleStarted;

    UPROPERTY(BlueprintAssignable)
    FVoidPlayerEvent OnShuffleEnded;

protected:

    UPROPERTY(Transient, BlueprintReadOnly)
    TArray<UCardWidget*> DeckCards;

    UPROPERTY(Transient, BlueprintReadOnly)
    TArray<UCardWidget*> DiscardCards;

    UPROPERTY(Transient, BlueprintReadOnly)
    TArray<UCardWidget*> ShuffleCards;

    UPROPERTY(Transient, BlueprintReadOnly)
    TArray<UCardWidget*> HandCards;

};
