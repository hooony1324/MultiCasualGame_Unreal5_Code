// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PropSpawnPoint.generated.h"

UCLASS()
class MULTICASUAL_API APropSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APropSpawnPoint();

	UFUNCTION(BlueprintCallable)
	void SpawnProp(int32 PropIndex);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<class AProp>> Props;

protected:
	virtual void BeginPlay() override;

	

public:	
	//virtual void Tick(float DeltaTime) override;

};
