// Copyright Epic Games, Inc. All Rights Reserved.


#include "FPSFrameworkWeaponComponent.h"
#include "FPSFrameworkCharacter.h"
#include "FPSFrameworkProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UFPSFrameworkWeaponComponent::UFPSFrameworkWeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}

bool UFPSFrameworkWeaponComponent::AttachWeapon(AFPSFrameworkCharacter* TargetCharacter)
{
	Character = TargetCharacter;

	// Check that the character is valid, and has no weapon component yet
	if (Character == nullptr || Character->GetInstanceComponents().FindItemByClass<UFPSFrameworkWeaponComponent>())
	{
		return false;
	}

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));

	// Set up action bindings
	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			// Set the priority of the mapping to 1, so that it overrides the Jump action with the Fire action when using touch input
			Subsystem->AddMappingContext(WeaponMappingContext, 1);
		}

		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
		{
			// Fire
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &UFPSFrameworkWeaponComponent::Fire);

			//Reload
			EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &UFPSFrameworkWeaponComponent::Reload);

			// Aim
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &UFPSFrameworkWeaponComponent::StartAiming);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &UFPSFrameworkWeaponComponent::StopAiming);

			// Switch Fire Mode
			EnhancedInputComponent->BindAction(SwitchFireModeAction, ETriggerEvent::Triggered, this, &UFPSFrameworkWeaponComponent::SwitchFireMode);

			// Hold Breath
			EnhancedInputComponent->BindAction(HoldBreathAction, ETriggerEvent::Started, this, &UFPSFrameworkWeaponComponent::StartHoldBreath);
			EnhancedInputComponent->BindAction(HoldBreathAction, ETriggerEvent::Completed, this, &UFPSFrameworkWeaponComponent::StopHoldBreath);
		}
	}

	return true;
}

void UFPSFrameworkWeaponComponent::Fire()
{
	if (Character == nullptr || Character->GetController() == nullptr)
	{
		return;
	}

	// Try and fire a projectile
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// Spawn the projectile at the muzzle
			World->SpawnActor<AFPSFrameworkProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}

	// Try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}

	// Try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void UFPSFrameworkWeaponComponent::Reload()
{
	// Implement reload logic here
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Reload"));
}

void UFPSFrameworkWeaponComponent::StartAiming()
{
	// Implement aiming logic here
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Start Aiming"));
}

void UFPSFrameworkWeaponComponent::StopAiming()
{
	// Implement stop aiming logic here
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Stop Aiming"));
}

void UFPSFrameworkWeaponComponent::SwitchFireMode()
{
	// Implement fire mode switching logic here
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Switch Fire Mode"));
}

void UFPSFrameworkWeaponComponent::StartHoldBreath()
{
	// Implement hold breath logic here
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Start Holding Breath"));
}

void UFPSFrameworkWeaponComponent::StopHoldBreath()
{
	// Implement stop hold breath logic here
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Stop Holding Breath"));
}

void UFPSFrameworkWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// ensure we have a character owner
	if (Character != nullptr)
	{
		// remove the input mapping context from the Player Controller
		if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->RemoveMappingContext(WeaponMappingContext);
			}
		}
	}

	// maintain the EndPlay call chain
	Super::EndPlay(EndPlayReason);
}