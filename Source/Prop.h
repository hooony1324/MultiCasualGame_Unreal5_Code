// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Prop.generated.h"

UCLASS()
class MULTICASUAL_API AProp : public AActor
{
	GENERATED_BODY()
	
public:	
	AProp();
	friend class APropSpawnPoint;
	//virtual void Tick(float DeltaTime) override;
	void EnableCustomDepth(bool bEnable);
	void SetPropMeshAsset(USkeletalMesh* Mesh);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* SkeletalMeshComponent;

	UPROPERTY()
	USkeletalMesh* PropMesh;

private:
	FTimerHandle DisableCustomDepthTimer;
	FTimerDelegate DisableCustomDepthDelegate;

public:	
	USkeletalMesh* GetPropMesh();
};
