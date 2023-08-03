// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_Melee UMETA(DisplayName = "MeleeArms"),
	EWT_Fire UMETA(DisplayName = "FireArms"),

	EWT_MAX UMETA(DisplayName = "DefaultMAX")
};


UCLASS(Blueprintable)
class MULTICASUAL_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Attack(const FVector& HitTarget);

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;


protected:
	virtual void BeginPlay() override;
	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;

	virtual void OnEquipped() {};

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY()
	class AMultiCasualGameCharacter* OwnerCharacter;

	UPROPERTY()
	class AMultiCasualPlayerController* OwnerController;

private:


	//**
	// Zooming
	//**

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	//**
	// Weapon Setup
	//**


	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackDelay = .15f;

	//-> MeleeArms

	//-> FireArms
	//UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	//class UAnimationAsset* FireAnimation;

public:	

	float GetZoomedFOV() { return ZoomedFOV; }
	float GetZoomInterpSpeed() { return ZoomInterpSpeed; }
	float GetAttackDelay() { return AttackDelay; }
	EWeaponType GetWeaponType() const{ return WeaponType; }
};
