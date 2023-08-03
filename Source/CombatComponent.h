
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HideGameTeam.h"
#include "GameHUD.h"
#include "CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTICASUAL_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AMultiCasualGameCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerSpawnDefaultWeapon();

	void EquipWeapon(class AWeapon* WeaponToEquip);

	void AttackButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerWeaponButtonPressed();

protected:
	virtual void BeginPlay() override;

private:
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
	void SetHUDCrosshairs(float DeltaTime);

	//**
	// Hide Team
	//**
	void HideTeamUpdate(float DeltaTime, FHitResult HitResult);
	void TargettingProps(const FHitResult& TraceHitResult);


	//**
	// Seek Team
	//**
	void SeekTeamUpdate(float DeltaTime, FHitResult HitResult);

	//**
	// Weapon
	//**
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);
	
	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	void SwapWeapon();

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	void Attack();
	bool CanAttack();
	void StartAttackTimer();
	void AttackFinished();

	FTimerHandle AttackTimer;
	bool bCanAttack = true;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	// Networking WeaponAttack
	FVector HitTarget;

	UFUNCTION(Server, Reliable)
	void ServerAttack(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAttack(const FVector_NetQuantize& TraceHitTarget);

	void LocalAttack(const FVector_NetQuantize& TraceHitTarget);

private:
	UPROPERTY()
	class AMultiCasualGameCharacter* Character;

	UPROPERTY()
	class AMultiCasualPlayerController* Controller;

	UPROPERTY()
	class AGameHUD* GameHUD;

	FHUDPackage HUDPackage;

	UPROPERTY(BluePrintReadOnly, ReplicatedUsing = OnRep_EquippedWeapon, meta = (AllowPrivateAccess = "true"))
	class AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> MeleeWeaponClass;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class AWeapon* MeleeWeapon;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMesh* TargetingPropMesh = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bSeeking = false;

	ETeam CurrentTeam = ETeam::ET_NoTeam;

	bool bAttackButtonPressed = false;

	//**
	// Zoom
	//**
	void InterpFOV(float DeltaTime);

	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	bool bAiming = false;

public:	
	void SetSeeking(bool bIsSeeking) { bSeeking = bIsSeeking; }
	void SetCurrentTeam(ETeam Team) { CurrentTeam = Team; }

	UFUNCTION(BlueprintCallable)
	const ETeam GetCurrentTeam() const { return CurrentTeam; }

	void SetAiming(bool bIsAiming) { bAiming = bIsAiming; }

	UFUNCTION(BlueprintCallable)
	AWeapon* GetMeleeWeapon() { return MeleeWeapon; }
};
