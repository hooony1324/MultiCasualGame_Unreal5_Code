// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "MultiCasualGameCharacter.generated.h"

UCLASS()
class MULTICASUAL_API AMultiCasualGameCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMultiCasualGameCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintImplementableEvent)
	void ServerSetOverheadMessage(const FString& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void SetPropMesh(bool bToProp);

	void PlayAttackMontage(bool bIsMelee);
	void PlayHitReactMontage();
	
	void Elim(bool bPlayerLeftGame);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	UFUNCTION(BlueprintImplementableEvent)
	void StartSpectateCamera();

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Action
	void MouseLeftPressed();
	void MouseLeftReleased();
	void MouseRightPressed();
	void MouseRightReleased();
	void Q_Pressed();
	void E_Pressed();
	void Weapon1_Pressed();

	// Axis
	void MoveForward(float Value);
	void MoveRight(float Value);
	void MouseWheel(float Value);

private:

	//**
	// GameMode
	//**

	UPROPERTY()
	class AHideGameGameMode* HideGameMode;

	//**
	// Character
	//**

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UClass* AnimBPCharacter;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidgetComponent;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UOverheadWidget* OverheadWidget;
	
	//**
	// Prop Moking
	//**

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMesh* OriginalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMesh* TargetPropMesh = nullptr;

	//**
	// Animation Montages
	//**

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;

	void PlayElimMontage();

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* ElimMontage;

	//**
	// Player Health
	//**

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AMultiCasualPlayerController> PlayerController;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void UpdateHUDHealth();

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	//**
	// Dissolve
	//**

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	//**
	// Elim
	//**

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimSound;

	void ElimTimerFinished();

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 2.f;

	bool bElimmed = false;
	bool bLeftGame = false;

	//**
	// Using Game Instance
	//**
	
	UPROPERTY()
	TObjectPtr<class UMultiCasualGameInstance> MultiCasualGameInstance;

public:
	UOverheadWidget* GetOverheadWidget() const { return OverheadWidget; }
	UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	bool IsWeaponEquipped();
	UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	void SetElimmed(bool bIsElimmed) { bElimmed = bIsElimmed; };

	UFUNCTION(BlueprintCallable)
	void SetKeyInfoVisible(bool bIsVisible);

	UFUNCTION(BlueprintPure)
	bool GetKeyInfoVisible();
};
