

#include "PropSpawnPoint.h"
#include "Prop.h"

APropSpawnPoint::APropSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APropSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
}

void APropSpawnPoint::SpawnProp(int32 PropIndex)
{
	int32 NumPropMeshAssets = Props.Num();
	if (NumPropMeshAssets > 0)
	{
		//Props[0];
		//AProp* PropTest = GetWorld()->SpawnActor<AProp>(AProp::StaticClass(), GetActorTransform());
		//PropTest->SetPropMeshAsset();

		//AProp* Prop = GetWorld()->SpawnActor<AProp>(AProp::StaticClass(), GetActorTransform());
		//Prop->SetPropMeshAsset(PropMeshAssets[PropIndex]);
	}
}

