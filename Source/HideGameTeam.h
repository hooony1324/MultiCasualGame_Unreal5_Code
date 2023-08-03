#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	ET_HideTeam UMETA(DisplayName = "HideTeam"),
	ET_SeekTeam UMETA(DisplayName = "SeekTeam"),
	ET_NoTeam UMETA(DisplayName = "NoTeam"),
	
	
	ET_MAX UMETA(DisplayName = "DefaultMax")
};