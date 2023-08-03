// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeArms.h"
#include "Components/BoxComponent.h"
#include "MultiCasual.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/EngineTypes.h"
#include "Sound/SoundCue.h"
#include "MultiCasualGameCharacter.h"
#include "MultiCasualPlayerController.h"
#include "HideGamePlayerState.h"

AMeleeArms::AMeleeArms()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
}

void AMeleeArms::BeginPlay()
{
	Super::BeginPlay();

	
}

// Attack : ServerRPC -> MulticastRPC
// SapwnSound는 Multicast
// 데미지 처리는 서버에서
void AMeleeArms::Attack(const FVector& HitTarget)
{
	Super::Attack(HitTarget);

	OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMultiCasualGameCharacter>(GetOwner()) : OwnerCharacter;
	if (OwnerCharacter == nullptr)
	{
		return;
	}
	OwnerController = OwnerController == nullptr ? Cast<AMultiCasualPlayerController>(OwnerCharacter->GetInstigatorController()) : OwnerController;

	TArray<FHitResult> HitResults;

	FVector CharacterPos = OwnerCharacter->GetActorLocation();
	FVector TraceStart = CharacterPos + OwnerCharacter->GetActorForwardVector() * 100 + FVector(0, 0, -40);
	UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		TraceStart,
		TraceStart,
		50,
		UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn),
		false,
		TArray<AActor*, FDefaultAllocator>(),
		EDrawDebugTrace::None,
		HitResults,
		true
	);


	for (FHitResult HitResult : HitResults)
	{
		AController* InstigatorController = Owner->GetInstigatorController();
		
		if (InstigatorController)
		{
			AMultiCasualGameCharacter* OtherCharacter = Cast<AMultiCasualGameCharacter>(HitResult.GetActor());
			if (OtherCharacter && OtherCharacter != OwnerCharacter)
			{
				if (ImpactSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
				}

				if (OwnerCharacter->HasAuthority())
				{
					UGameplayStatics::ApplyDamage(OtherCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
				}

				return;
			}
		}
	}

}
