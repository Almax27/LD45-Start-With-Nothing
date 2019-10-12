// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlayerController.h"
#include "CardWidget.h"
#include "Util.h"

void AGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

    for (auto CardClass : Deck)
    {
        if (auto CardWidget = CreateWidget<UCardWidget>(this, CardClass))
        {
            DeckCards.Add(CardWidget);
        }
    }

    Shuffle(DeckCards);
}

void AGamePlayerController::ResetCards()
{
    DiscardAll();
    DeckCards.Append(DiscardCards);
    DiscardCards.Reset();
}

UCardWidget* AGamePlayerController::DrawCard()
{
    if (DeckCards.Num() > 0)
    {
        UCardWidget* Card = nullptr;
        if (HandCards.Num() == 0)
        {
            if (auto CardPtr = DeckCards.FindByPredicate([](const UCardWidget* Card) { return Card->IsUnitCard; }))
            {
                Card = *CardPtr;
                DeckCards.Remove(Card);
            }
        }
        if(!Card)
        {
            Card = DeckCards.Pop(false);
        }
        check(Card);

        HandCards.Add(Card);

        Card->Drawn();
        OnCardDrawn.Broadcast(this, Card);

        return Card;
    }
    return nullptr;
}

bool AGamePlayerController::DiscardCard(UCardWidget* Card)
{
    if (Card && HandCards.Contains(Card))
    {
        HandCards.Remove(Card);
        DiscardCards.Add(Card);

        Card->Discarded();
        OnCardDiscarded.Broadcast(this, Card);

        return true;
    }
    return false;
}

void AGamePlayerController::DiscardAll()
{
    auto CardsToDiscard = HandCards;
    for (auto Card : CardsToDiscard)
    {
        DiscardCard(Card);
    }
}

void AGamePlayerController::StartShuffle()
{
    ShuffleCards = DiscardCards;
    DiscardCards.Reset();
    OnShuffleStarted.Broadcast(this);
}

void AGamePlayerController::EndShuffle()
{
    DeckCards.Append(ShuffleCards);
    Shuffle(DeckCards);
    OnShuffleEnded.Broadcast(this);
}