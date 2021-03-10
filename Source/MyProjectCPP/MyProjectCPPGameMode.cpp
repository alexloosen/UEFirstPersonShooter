// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyProjectCPPGameMode.h"
#include "MyProjectCPPHUD.h"
#include "MyProjectCPPCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMyProjectCPPGameMode::AMyProjectCPPGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AMyProjectCPPHUD::StaticClass();
}
