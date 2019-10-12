// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Util.h"
#include "MapEntity.generated.h"

class AHexCell;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMapEntityDelegate, class AMapEntity*, MapEntity);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMapEntityMoveDelegate, class AMapEntity*, MapEntity, AHexCell*, FromCell, AHexCell*, ToCell);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMapEntityAttackDelegate, class AMapEntity*, MapEntity, const FMapAttackInfo&, AttackInfo);

UCLASS(Abstract, Blueprintable)
class LD45_API AMapEntity : public AActor
{
	GENERATED_BODY()
	
public:	

	AMapEntity();

protected:
    void BeginPlay() override;
    void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:

    UFUNCTION(BlueprintCallable)
    bool GetIsFriendly() const { return IsFriendly; }

    UFUNCTION(BlueprintCallable)
    bool GetIsHighValue() const { return IsHighValue; }

    UFUNCTION(BlueprintCallable)
    AHexCell* GetMapCell() const { return MapCell; }

    UFUNCTION(BlueprintCallable)
    bool MoveToMapCell(AHexCell* Cell);

    UFUNCTION(BlueprintCallable)
    bool PerformAttack(const FMapAttackInfo& AttackInfo);

    UFUNCTION(BlueprintCallable)
    bool TryPerformAttack(AHexCell* Cell, const TArray<FMapAttackInfo>& Attacks);

    UFUNCTION(BlueprintCallable)
    TArray<FHexMapCoord> GetMoveLocations() const;

    UFUNCTION(BlueprintCallable)
    TArray<FMapAttackInfo> GetAttacks() const;

public:

    UFUNCTION(BlueprintCallable)
    bool AIMove();

    UFUNCTION(BlueprintCallable)
    bool AITelegraphAttack();

    UFUNCTION(BlueprintCallable)
    bool AIResolveAttack();

    UFUNCTION(BlueprintCallable)
    bool GetAIHasAttackPending() const { return AIHasAttackPending; }

protected:

    bool AIHasAttackPending = false;

    UPROPERTY(Transient)
    FMapAttackInfo PendingAttackInfo;

protected:

    UPROPERTY(EditDefaultsOnly)
    bool IsFriendly;

    UPROPERTY(EditDefaultsOnly)
    bool IsHighValue;

    UPROPERTY(EditDefaultsOnly)
    int MoveDistance;

    UPROPERTY(EditDefaultsOnly)
    int AttackDistance;

    UPROPERTY(BlueprintAssignable)
    FMapEntityMoveDelegate OnMove;

    UPROPERTY(BlueprintAssignable)
    FMapEntityAttackDelegate OnAttack;

    UPROPERTY(Transient)
    AHexCell* MapCell;

public:

    float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    UFUNCTION(BlueprintCallable)
    bool Kill();

    UFUNCTION(BlueprintCallable)
    int GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable)
    int GetHealth() const { return Health; }

    UFUNCTION(BlueprintCallable)
    void SetHealth(int NewHealth);

protected:

    UFUNCTION(BlueprintNativeEvent)
    void HealthChanged();

    UFUNCTION(BlueprintNativeEvent)
    void Death();

public:

    UPROPERTY(BlueprintAssignable)
    FMapEntityDelegate OnHealthChanged;

    UPROPERTY(BlueprintAssignable)
    FMapEntityDelegate OnDeath;

protected:

    UPROPERTY(EditDefaultsOnly)
    int MaxHealth;

    UPROPERTY(Transient, VisibleInstanceOnly)
    int Health;

};
