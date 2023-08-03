

#include "Weapon.h"
#include "MultiCasualGameCharacter.h"
#include "MultiCasualPlayerController.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMesh");
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		OwnerCharacter = nullptr;
		OwnerController = nullptr;
	}
	else
	{
		OwnerCharacter = OwnerCharacter == nullptr ? Cast<AMultiCasualGameCharacter>(Owner) : OwnerCharacter;
		if (OwnerCharacter)
		{
			OwnerController = Cast<AMultiCasualPlayerController>(OwnerCharacter->GetController());
			// ���� �ٲ���� �� HUD���ÿ��� �̿�
		}
	}
}


void AWeapon::Attack(const FVector& HitTarget)
{
	// ���� ����
	// WeaponMesh->Animation Fire, Sound ����

}

