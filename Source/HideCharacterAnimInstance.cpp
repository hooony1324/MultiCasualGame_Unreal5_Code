// Fill out your copyright notice in the Description page of Project Settings.


#include "HideCharacterAnimInstance.h"
#include "MultiCasualGameCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UHideCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	GameCharacter = Cast<AMultiCasualGameCharacter>(TryGetPawnOwner());


}

void UHideCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (GameCharacter == nullptr)
	{
		GameCharacter = Cast<AMultiCasualGameCharacter>(TryGetPawnOwner());
	}
	if (GameCharacter == nullptr) return;

	FVector Velocity = GameCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	//**
	//	Update bool Settings
	//**
	bIsInAir = GameCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = GameCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = GameCharacter->IsWeaponEquipped();
	bElimmed = GameCharacter->IsElimmed();
}
