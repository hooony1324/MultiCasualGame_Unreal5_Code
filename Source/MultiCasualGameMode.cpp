// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiCasualGameMode.h"
#include "MultiCasualGameCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "MultiCasualPlayerController.h"

#include "GameFramework/Character.h"

AMultiCasualGameMode::AMultiCasualGameMode()
{
	static ConstructorHelpers::FClassFinder<ACharacter> PlayerPawnBPClass(TEXT("/Game/Blueprints/Character/BP_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}