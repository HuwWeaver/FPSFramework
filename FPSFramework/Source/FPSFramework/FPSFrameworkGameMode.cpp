// Copyright Epic Games, Inc. All Rights Reserved.

#include "FPSFrameworkGameMode.h"
#include "FPSFrameworkCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPSFrameworkGameMode::AFPSFrameworkGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
