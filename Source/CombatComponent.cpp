

#include "CombatComponent.h"
#include "MultiCasualGameCharacter.h"
#include "MultiCasualPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MultiCasual.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "OverheadWidget.h"
#include "Prop.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;


}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, CombatState);
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		// 초기화 관련
		Character->GetRootComponent();

		// FOV
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsElimmed())
	{
		return;
	}

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		InterpFOV(DeltaTime);

		switch (CurrentTeam)
		{
		case ETeam::ET_HideTeam:
			HideTeamUpdate(DeltaTime, HitResult);
			break;
		case ETeam::ET_SeekTeam:
			SeekTeamUpdate(DeltaTime, HitResult);
			break;
		case ETeam::ET_NoTeam:
			break;
		default:
			break;
		}
	}
}

void UCombatComponent::AttackButtonPressed(bool bPressed)
{
	if (Character == nullptr)
	{
		return;
	}

	bAttackButtonPressed = bPressed;

	if (bAttackButtonPressed)
	{
		if (CurrentTeam == ETeam::ET_HideTeam)
		{
			Character->SetPropMesh(true);
		}
		else if (CurrentTeam == ETeam::ET_SeekTeam)
		{
			Attack();
		}
	}

}

void UCombatComponent::HideTeamUpdate(float DeltaTime, FHitResult HitResult)
{
	TargettingProps(HitResult);
}


void UCombatComponent::SeekTeamUpdate(float DeltaTime, FHitResult HitResult)
{
	SetHUDCrosshairs(DeltaTime);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	if (EquippedWeapon == WeaponToEquip) return;

	if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;

}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;

	// TODO
	// 1. 이미 첫번째 무기 있다면
	// 2. 무기 가 꽉 찼다면

	EquippedWeapon = WeaponToEquip;
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	AttachActorToBackpack(WeaponToEquip);
	SecondaryWeapon->SetOwner(Character);
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::SwapWeapon()
{
	if (Character == nullptr || !Character->HasAuthority())
	{
		return;
	}

	CombatState = ECombatState::ECS_SwappingWeapons;

	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	AttachActorToRightHand(EquippedWeapon);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::ServerSpawnDefaultWeapon_Implementation()
{
	UWorld* World = GetWorld();
	if (World && MeleeWeaponClass)
	{
		MeleeWeapon = World->SpawnActor<AWeapon>(MeleeWeaponClass);
		EquipSecondaryWeapon(MeleeWeapon);
	}
}

void UCombatComponent::ServerWeaponButtonPressed_Implementation()
{
	if (EquippedWeapon == nullptr && SecondaryWeapon == nullptr)
	{
		return;
	}

	SwapWeapon();
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		AttachActorToRightHand(EquippedWeapon);
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		AttachActorToBackpack(SecondaryWeapon);
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		break;
	case ECombatState::ECS_SwappingWeapons:
		if (Character && !Character->IsLocallyControlled())
		{
			SwapWeapon();
			CombatState = ECombatState::ECS_Unoccupied;
		}
		break;
	}
}


void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);


	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			// 레이저 캐릭터보다 살짝 앞에서 시작
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 10.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		//DrawDebugLine(
		//	GetWorld(),
		//	Start,
		//	End,
		//	FColor::Purple
		//);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}

	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<AMultiCasualPlayerController>(Character->GetController()) : Controller;

	if (Controller)
	{
		GameHUD = GameHUD == nullptr ? Cast<AGameHUD>(Controller->GetHUD()) : GameHUD;
		if (GameHUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsColor = FLinearColor::White;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}

			GameHUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (bAiming && EquippedWeapon)
	{
		float TargetFOV = EquippedWeapon->GetZoomedFOV();
		float ZoomSpeed = EquippedWeapon->GetZoomInterpSpeed();
		CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, ZoomSpeed);
	}
	else if (bAiming && !EquippedWeapon)
	{
		float TargetFOV = ZoomedFOV;
		float ZoomSpeed = ZoomInterpSpeed;
		CurrentFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, ZoomSpeed);
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::TargettingProps(const FHitResult& TraceHitResult)
{

	FHitResult SphereHitResult;
	UKismetSystemLibrary::SphereTraceSingle(
		GetWorld(),
		TraceHitResult.ImpactPoint,
		TraceHitResult.ImpactPoint,
		20,
		UEngineTypes::ConvertToTraceType(ECC_PROPS),
		false,
		TArray<AActor*, FDefaultAllocator>(),
		EDrawDebugTrace::Type::None,
		SphereHitResult,
		true
	);

	if (SphereHitResult.bBlockingHit)
	{
		AProp* TargetProp = Cast<AProp>(SphereHitResult.GetActor());

		if (TargetProp)
		{
			TargetingPropMesh = TargetProp->GetPropMesh();
			TargetProp->EnableCustomDepth(true);

		}
		else
		{
			TargetingPropMesh = nullptr;
		}
	}
	else
	{
		TargetingPropMesh = nullptr;
	}
}

void UCombatComponent::Attack()
{
	if (CanAttack())
	{
		bCanAttack = false;
		if (EquippedWeapon)
		{
			ServerAttack(HitTarget);	
		}

		StartAttackTimer();
	}
}

void UCombatComponent::LocalAttack(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	Character->PlayAttackMontage(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Melee);
	EquippedWeapon->Attack(TraceHitTarget);
}

void UCombatComponent::ServerAttack_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastAttack(TraceHitTarget);
}

void UCombatComponent::MulticastAttack_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	LocalAttack(TraceHitTarget);
}

bool UCombatComponent::CanAttack()
{
	if (EquippedWeapon == nullptr)
	{
		return false;
	}

	return bCanAttack;
}

void UCombatComponent::StartAttackTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr)
	{
		return;
	}

	Character->GetWorldTimerManager().SetTimer(
		AttackTimer,
		this,
		&UCombatComponent::AttackFinished,
		EquippedWeapon->GetAttackDelay()
	);
}

void UCombatComponent::AttackFinished()
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	bCanAttack = true;

}
