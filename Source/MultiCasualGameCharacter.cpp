

#include "MultiCasualGameCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MultiCasualPlayerController.h"
#include "GameHUD.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "CombatComponent.h"
#include "Components/WidgetComponent.h"
#include "OverheadWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "MultiCasual.h"
#include "Net/UnrealNetwork.h"
#include "HideGamePlayerState.h"
#include "HideGameGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "MultiCasualGameInstance.h"

AMultiCasualGameCharacter::AMultiCasualGameCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 100.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	JumpMaxCount = 2;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 0, 850.f);
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.6f;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_PROPS, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_PROPS, ECollisionResponse::ECR_Ignore);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));
	CombatComponent->SetIsReplicated(true);

	OverheadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidgetComponent->SetupAttachment(RootComponent);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void AMultiCasualGameCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->Character = this;

	}
}

void AMultiCasualGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<AMultiCasualPlayerController>(Controller);
	OriginalMesh = GetMesh()->GetSkeletalMeshAsset();
	AnimBPCharacter = GetMesh()->GetAnimClass();
	bElimmed = false;
	OverheadWidget = Cast<UOverheadWidget>(OverheadWidgetComponent->GetUserWidgetObject());

	MultiCasualGameInstance = Cast<UMultiCasualGameInstance>(GetGameInstance());


	// Prop Moking
	GetMesh()->SetRelativeScale3D(FVector::One() * SQUIZSCALE);

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMultiCasualGameCharacter::ReceiveDamage);
	}
}


void AMultiCasualGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiCasualGameCharacter, Health);

}

void AMultiCasualGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void AMultiCasualGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Action
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("MouseLeft", IE_Pressed, this, &AMultiCasualGameCharacter::MouseLeftPressed);
	PlayerInputComponent->BindAction("MouseLeft", IE_Released, this, &AMultiCasualGameCharacter::MouseLeftReleased);
	PlayerInputComponent->BindAction("MouseRight", IE_Pressed, this, &AMultiCasualGameCharacter::MouseRightPressed);
	PlayerInputComponent->BindAction("MouseRight", IE_Released, this, &AMultiCasualGameCharacter::MouseRightReleased);
	PlayerInputComponent->BindAction("Q", IE_Pressed, this, &AMultiCasualGameCharacter::Q_Pressed);
	PlayerInputComponent->BindAction("E", IE_Pressed, this, &AMultiCasualGameCharacter::E_Pressed);
	PlayerInputComponent->BindAction("Weapon1", IE_Pressed, this, &AMultiCasualGameCharacter::Weapon1_Pressed);

	// Axis
	PlayerInputComponent->BindAxis("MoveForward", this, &AMultiCasualGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMultiCasualGameCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("MouseWheel", this, &AMultiCasualGameCharacter::MouseWheel);

}


void AMultiCasualGameCharacter::MouseLeftPressed()
{
	if (CombatComponent)
	{
		CombatComponent->AttackButtonPressed(true);

		// MokingProp은 Blueprint에서 진행되고 있음
	}
}

void AMultiCasualGameCharacter::MouseLeftReleased()
{
	if (CombatComponent)
	{
		//Combat->button...(false)
	}
}

void AMultiCasualGameCharacter::MouseRightPressed()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void AMultiCasualGameCharacter::MouseRightReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void AMultiCasualGameCharacter::Q_Pressed()
{
	if (CombatComponent)
	{
		SetPropMesh(false);
	}
}

void AMultiCasualGameCharacter::E_Pressed()
{
	if (CombatComponent)
	{
		SetPropMesh(true);
	}
}

void AMultiCasualGameCharacter::Weapon1_Pressed()
{
	if (CombatComponent)
	{
		CombatComponent->ServerWeaponButtonPressed();
	}
}


void AMultiCasualGameCharacter::MoveForward(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMultiCasualGameCharacter::MoveRight(float Value)
{
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AMultiCasualGameCharacter::MouseWheel(float Value)
{
	if (!bElimmed)
	{
		return;
	}

	if (Value > 0)
	{
		PlayerController->SepectateNextPlayer();
	}
	else if (Value < 0)
	{
		PlayerController->SepectatePrevPlayer();
	}
}

bool AMultiCasualGameCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}


void AMultiCasualGameCharacter::PlayAttackMontage(bool bIsMelee)
{
	// bMelee, bAiming(추후예정)
	if (!IsWeaponEquipped())
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);
		FName SectionName;
		SectionName = bIsMelee ? FName("MeleeAttack") : FName("FireAttack");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMultiCasualGameCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("HitReact_Front");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMultiCasualGameCharacter::Elim(bool bPlayerLeftGame)
{
	MulticastElim(bPlayerLeftGame);
}

void AMultiCasualGameCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AMultiCasualGameCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AMultiCasualGameCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AMultiCasualGameCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	//**
	// Prop to Character
	//**
	GetMesh()->SetSkeletalMeshAsset(OriginalMesh);
	GetMesh()->SetAnimInstanceClass(AnimBPCharacter);
	GetMesh()->SetRelativeScale3D(FVector::OneVector * 0.5f);

	bLeftGame = bPlayerLeftGame;
	if (PlayerController)
	{
		// sethud
	}
	bElimmed = true;
	PlayElimMontage();

	//**
	// Start Dissolve
	//**
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), -0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 50.f);
	}
	StartDissolve();

	//**
	// Disable Gameplay
	//**
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	if (CombatComponent)
	{
		CombatComponent->AttackButtonPressed(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//**
	// Elim Process
	//**
	if (ElimSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimSound,
			GetActorLocation()
		);
	}

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AMultiCasualGameCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void AMultiCasualGameCharacter::ElimTimerFinished()
{
	HideGameMode = HideGameMode == nullptr ? GetWorld()->GetAuthGameMode<AHideGameGameMode>() : HideGameMode;
	if (HideGameMode && !bLeftGame)
	{
		// 관전
		StartSpectateCamera();
	}
	if (bLeftGame && IsLocallyControlled())
	{
		// TODO: LeftGame을 Broadcast

	}
}

void AMultiCasualGameCharacter::PlayElimMontage()
{
	if (CombatComponent == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
		FName SectionName("Elim");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

// 서버에서만 호출
void AMultiCasualGameCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	HideGameMode = HideGameMode == nullptr ? GetWorld()->GetAuthGameMode<AHideGameGameMode>() : HideGameMode;

	if (HideGameMode == nullptr)
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth); // OnRep_Health호출됨
	
	PlayHitReactMontage();
	UpdateHUDHealth();

	if (Health == 0.f)
	{
		if (HideGameMode)
		{
			PlayerController = PlayerController == nullptr ? Cast<AMultiCasualPlayerController>(Controller) : PlayerController;
			AMultiCasualPlayerController* AttackerController = Cast<AMultiCasualPlayerController>(InstigatorController);
			HideGameMode->PlayerEliminated(this, PlayerController, AttackerController);

		}
	}
}

void AMultiCasualGameCharacter::UpdateHUDHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<AMultiCasualPlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void AMultiCasualGameCharacter::OnRep_Health()
{
	PlayHitReactMontage();
	UpdateHUDHealth();
}

void AMultiCasualGameCharacter::SetKeyInfoVisible(bool bIsVisible)
{
	MultiCasualGameInstance = MultiCasualGameInstance == nullptr ? Cast<UMultiCasualGameInstance>(GetGameInstance()) : MultiCasualGameInstance;

	if (MultiCasualGameInstance == nullptr)
	{
		return;
	}

	if (bIsVisible)
	{
		MultiCasualGameInstance->SetKeyInfoVisible(true);
	}
	else
	{
		MultiCasualGameInstance->SetKeyInfoVisible(false);
	}
}

bool AMultiCasualGameCharacter::GetKeyInfoVisible()
{
	MultiCasualGameInstance = MultiCasualGameInstance == nullptr ? Cast<UMultiCasualGameInstance>(GetGameInstance()) : MultiCasualGameInstance;

	if (MultiCasualGameInstance == nullptr)
	{
		return false;
	}

	return MultiCasualGameInstance->GetKeyInfoVisible();
}
