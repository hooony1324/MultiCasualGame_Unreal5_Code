// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "FireArms.generated.h"

/**
 * 
 */
UCLASS()
class MULTICASUAL_API AFireArms : public AWeapon
{
	GENERATED_BODY()
	
public:

protected:
	virtual void Attack(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;
};
