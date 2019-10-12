#pragma once

#include "CoreMinimal.h"
#include "Util.generated.h"

#define GETENUMSTRING(etype, evalue) \
( (FindObject<UEnum>(ANY_PACKAGE, TEXT(#etype), true) != nullptr) ? FindObject<UEnum>(ANY_PACKAGE, TEXT(#etype), true)->GetNameStringByIndex((int32)evalue) : FString::Format(TEXT("Failed to resolve {0} as {1}"), {(int32)evalue, TEXT(#etype)}) )

class AMapEntity;

USTRUCT(BlueprintType)
struct FHexMapCoord
{
    GENERATED_BODY()

public:

    FHexMapCoord(int _x = 0, int _y = 0) : x(_x), y(_y) {}

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int x;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int y;

    FORCEINLINE bool IsValid(int MapWidth, int MapHeight)
    {
        return x >= 0 && x < MapWidth && y >= 0 && y < MapHeight;
    }

    FORCEINLINE bool operator == (const FHexMapCoord& rhs) const
    {
        return this->x == rhs.x && this->y == rhs.y;
    }

    FORCEINLINE FHexMapCoord operator * (const float rhs) const
    {
        FHexMapCoord lhs(*this);
        lhs.x *= rhs;
        lhs.y *= rhs;
        return lhs;
    }

    FORCEINLINE void operator *= (const float rhs)
    {
        *this = *this * rhs;
    }

    FORCEINLINE FHexMapCoord operator + (const FHexMapCoord& rhs) const
    {
        FHexMapCoord lhs(*this);
        lhs.x += rhs.x;
        lhs.y += rhs.y;
        return lhs;
    }

    FORCEINLINE void operator += (const FHexMapCoord& rhs)
    {
        *this = *this + rhs;
    }

    FORCEINLINE FHexMapCoord operator - (const FHexMapCoord& rhs) const
    {
        FHexMapCoord lhs(*this);
        lhs.x -= rhs.x;
        lhs.y -= rhs.y;
        return lhs;
    }

    FORCEINLINE void operator -= (const FHexMapCoord& rhs)
    {
        *this = *this - rhs;
    }
};

static const TArray<FHexMapCoord>& GetDirectionsAt(const FHexMapCoord& Coord)
{
    static const TArray<FHexMapCoord> EvenDirections = { {0,-1}, {1,0}, {0,1}, {-1,1}, {-1,0}, {-1,-1} };
    static const TArray<FHexMapCoord> OddDirections = { {1,-1}, {1,0}, {1,1}, {0,1}, {-1,0}, {0,-1} };
    return (Coord.y % 2) == 0 ? EvenDirections : OddDirections;
}

USTRUCT(BlueprintType)
struct FMapAttackInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FHexMapCoord> Locations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int Damage = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AMapEntity* Source = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AMapEntity*> Hits;
};

template<typename T> 
static void Shuffle(TArray<T>& Array)
{
    int32 LastIndex = Array.Num() - 1;
    for (int32 i = 0; i < LastIndex; ++i)
    {
        int32 Index = FMath::RandRange(0, LastIndex);
        if (i != Index)
        {
            Array.Swap(i, Index);
        }
    }
}