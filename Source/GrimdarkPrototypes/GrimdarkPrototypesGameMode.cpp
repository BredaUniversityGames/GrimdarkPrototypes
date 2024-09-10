// Copyright Epic Games, Inc. All Rights Reserved.

#include "GrimdarkPrototypesGameMode.h"
#include "GrimdarkPrototypesCharacter.h"
#include "UObject/ConstructorHelpers.h"

AGrimdarkPrototypesGameMode::AGrimdarkPrototypesGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
