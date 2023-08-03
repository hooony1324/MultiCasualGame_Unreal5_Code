

#include "Prop.h"
#include "Components/SkeletalMeshComponent.h"
#include "MultiCasual.h"
#include "GameFramework/Actor.h"

#define CUSTOM_DEPTH_BLUE 251

AProp::AProp()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_PROPS, ECollisionResponse::ECR_Block);

	SkeletalMeshComponent->SetSimulatePhysics(true);
	SkeletalMeshComponent->SetEnableGravity(true);
	SkeletalMeshComponent->BodyInstance.bLockXRotation = true;
	SkeletalMeshComponent->BodyInstance.bLockYRotation = true;
	SkeletalMeshComponent->BodyInstance.bLockZRotation = true;

	// OUTLINE EFFECT
	SkeletalMeshComponent->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	SkeletalMeshComponent->MarkRenderStateDirty();


	// Sync Physics Simulate
}

void AProp::BeginPlay()
{
	Super::BeginPlay();
	
	DisableCustomDepthDelegate.BindLambda([=]
		{
			EnableCustomDepth(false);
		}
	);
}

void AProp::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(DisableCustomDepthTimer);
}

void AProp::SetPropMeshAsset(USkeletalMesh* Mesh)
{
	SkeletalMeshComponent->SetSkeletalMeshAsset(Mesh);
	PropMesh = SkeletalMeshComponent->GetSkeletalMeshAsset();

	EnableCustomDepth(false);
}


USkeletalMesh* AProp::GetPropMesh()
{
	return PropMesh = PropMesh == nullptr ? SkeletalMeshComponent->GetSkeletalMeshAsset() : PropMesh;
}

void AProp::EnableCustomDepth(bool bEnable)
{
	if (SkeletalMeshComponent == nullptr)
	{
		return;
	}

	SkeletalMeshComponent->SetRenderCustomDepth(bEnable);

	if (bEnable)
	{
		GetWorld()->GetTimerManager().SetTimer(
			DisableCustomDepthTimer,
			DisableCustomDepthDelegate,
			0.1f,
			false
		);
	}
}


