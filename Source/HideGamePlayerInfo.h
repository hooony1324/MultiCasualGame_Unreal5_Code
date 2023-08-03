#pragma once

#include "HideGamePlayerInfo.generated.h"

USTRUCT(BlueprintType)
struct FPlayerInfo
{
	GENERATED_USTRUCT_BODY()

	FORCEINLINE bool operator==(const FPlayerInfo& Other) const
	{
		return this->PlayerName == Other.PlayerName;
	}

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	float PlayerScore;
};