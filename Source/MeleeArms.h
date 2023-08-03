// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "MeleeArms.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API AMeleeArms : public AWeapon
{
	GENERATED_BODY()
public:
	AMeleeArms();

protected:
	virtual void BeginPlay() override;
	virtual void Attack(const FVector& HitTarget) override;

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

private:
	
};
