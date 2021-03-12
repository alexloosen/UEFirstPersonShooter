// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyProjectCPPCharacter.h"
#include "MyProjectCPPProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AMyProjectCPPCharacter

AMyProjectCPPCharacter::AMyProjectCPPCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;

	HoldingComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HoldingComponent"));
	FVector vec = { 50.0f, 0.0f, 0.0f };
	HoldingComponent->SetRelativeLocation(vec);
	HoldingComponent->SetupAttachment(FP_MuzzleLocation);

	CurrentItem = NULL;
	bCanMove = true;
	bInspecting = false;

	currentWeapon = 0;
}

void AMyProjectCPPCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Starting Health: %i"), (int32)health);
	UE_LOG(LogTemp, Warning, TEXT("Starting Armor: %i"), (int32)armor);

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.

	PitchMin = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin;
	PitchMax = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax;
}

void AMyProjectCPPCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Start = FirstPersonCameraComponent->GetComponentLocation();
	ForwardVector = FirstPersonCameraComponent->GetForwardVector();
	End = ((ForwardVector * 200.0f) + Start);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);

	if(!bHoldingItem)
	{
		if(GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, DefaultComponentQueryParams, DefaultResponseParams))
		{
			if(Hit.GetActor()->GetClass()->IsChildOf(APickUp::StaticClass()))
			{
				CurrentItem = Cast<APickUp>(Hit.GetActor());
			}
		}
		else
		{
			CurrentItem = NULL;
		}
	}

	if(bInspecting)
	{
		if(bHoldingItem)
		{
			FirstPersonCameraComponent->SetFieldOfView(FMath::Lerp(FirstPersonCameraComponent->FieldOfView, 90.0f, 0.1f));
			HoldingComponent->SetRelativeLocation(FVector(0.0f, 50.0f, 50.0f));
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax = 179.90000002f;
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin = -179.90000002f;
			CurrentItem->rotateActor();
		}
		else
		{
			FirstPersonCameraComponent->SetFieldOfView(FMath::Lerp(FirstPersonCameraComponent->FieldOfView, 45.0f, 0.1f));
		}
	}
	else
	{
		FirstPersonCameraComponent->SetFieldOfView(FMath::Lerp(FirstPersonCameraComponent->FieldOfView, 90.0f, 0.1f));

		if(bHoldingItem)
		{
			HoldingComponent->SetRelativeLocation(FVector(0.0f, 50.0f, 0.0f));
		}
	}
	// Interpolate Health
	if(interpolatedHealth != health)
	{
		interpolatedHealth = FMath::FInterpTo(interpolatedHealth, health, 10.0f, DeltaSeconds);
		newHealthPercent = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, maxHealth), FVector2D(0.0f, 1.0f), interpolatedHealth);

		interpolatedArmor = FMath::FInterpTo(interpolatedArmor, armor, 10.0f, DeltaSeconds);
		newArmorPercent = FMath::GetMappedRangeValueClamped(FVector2D(0.0f, maxArmor), FVector2D(0.0f, 1.0f), interpolatedArmor);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMyProjectCPPCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMyProjectCPPCharacter::OnFire);

	PlayerInputComponent->BindAction("Action", IE_Pressed, this, &AMyProjectCPPCharacter::OnAction);

	PlayerInputComponent->BindAction("Inspect", IE_Pressed, this, &AMyProjectCPPCharacter::OnInspect);
	PlayerInputComponent->BindAction("Inspect", IE_Released, this, &AMyProjectCPPCharacter::OnInspectReleased);

	PlayerInputComponent->BindAction<weaponIndex>("Weapon1", IE_Pressed, this, &AMyProjectCPPCharacter::SwitchToWeapon, 0);
	PlayerInputComponent->BindAction<weaponIndex>("Weapon2", IE_Pressed, this, &AMyProjectCPPCharacter::SwitchToWeapon, 1);


	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AMyProjectCPPCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyProjectCPPCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyProjectCPPCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyProjectCPPCharacter::LookUpAtRate);
}

void AMyProjectCPPCharacter::SwitchToWeapon(int index)
{
	UE_LOG(LogTemp, Warning, TEXT("Switching to Weapon %i"), index);
	switch(index)
	{
		case 0:
			currentWeapon = 0;
			SwitchWeaponMesh(index);
			break;
		case 1:
			currentWeapon = 1;
			SwitchWeaponMesh(index);
			break;
		default:
			break;
	}
}

void AMyProjectCPPCharacter::OnFire()
{
	// try and fire a projectile
	if(ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if(World != nullptr)
		{
			if(weapons[currentWeapon])
			{
				if(weapons[currentWeapon]->ammo > 0)
				{
					const FRotator SpawnRotation = GetControlRotation();
					// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
					const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

					//Set Spawn Collision Handling Override
					FActorSpawnParameters ActorSpawnParams;
					ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

					// spawn the projectile at the muzzle
					World->SpawnActor<AMyProjectCPPProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
					weapons[currentWeapon]->ammo--;
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Out of ammo!"));
				}
			}
		}
	}

	// try and play the sound if specified
	if(FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if(FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if(AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AMyProjectCPPCharacter::OnAction()
{
	if(CurrentItem && !bInspecting)
	{
		ToggleItemPickup();
	}
}

void AMyProjectCPPCharacter::OnInspect()
{
	if(bHoldingItem)
	{
		LastRotation = GetControlRotation();
		ToggleMovement();
	}
	else
	{
		bInspecting = true;
	}
}

void AMyProjectCPPCharacter::OnInspectReleased()
{
	if(bInspecting && bHoldingItem)
	{
		GetController()->SetControlRotation(LastRotation);
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMax = PitchMax;
		GetWorld()->GetFirstPlayerController()->PlayerCameraManager->ViewPitchMin = PitchMin;
		ToggleMovement();
	}
	else
	{
		bInspecting = false;
	}
}

void AMyProjectCPPCharacter::ToggleMovement()
{
	bCanMove = !bCanMove;
	bInspecting = !bInspecting;
	FirstPersonCameraComponent->bUsePawnControlRotation = !FirstPersonCameraComponent->bUsePawnControlRotation;
	bUseControllerRotationYaw = !bUseControllerRotationYaw;
}

void AMyProjectCPPCharacter::ToggleItemPickup()
{
	if(CurrentItem)
	{
		bHoldingItem = !bHoldingItem;
		CurrentItem->PickUp();

		if(!bHoldingItem)
		{
			CurrentItem = NULL;
		}
	}
}

float AMyProjectCPPCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Warning, TEXT("I'm taking damage, OW!"));
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("OUCH!"));   
	// Incoming damage is split 2/3 to armor, 1/3 to health
	float damageToArmor = roundf(DamageAmount * 0.67);
	float damageToHealth = roundf(DamageAmount * 0.33);
	float armorDamageOverlap = 0.0f;

	if(armor > 0)
	{
		// if damage to armor is higher than player armor, set armor to 0, calculate overlap and apply overlap to health along with health damage
		if(armor < damageToArmor)
		{
			armorDamageOverlap = abs(armor - damageToArmor);
			armor = 0;

			health -= armorDamageOverlap;
			health -= damageToHealth;
		}
		// otherwise apply damage to both armor and health
		else
		{
			armor -= damageToArmor;
			health -= damageToHealth;
		}
	}
	// if player has no armor, apply full damage to health
	else
	{
		health -= DamageAmount;
	}
	UE_LOG(LogTemp, Warning, TEXT("Health: %i"), (int32)health);
	UE_LOG(LogTemp, Warning, TEXT("Armor: %i"), (int32)armor);

	if(health <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Health depleted!"));
	}
	return DamageAmount;
}

void AMyProjectCPPCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if(TouchItem.bIsPressed == true)
	{
		return;
	}
	if((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AMyProjectCPPCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if(TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AMyProjectCPPCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

void AMyProjectCPPCharacter::MoveForward(float Value)
{
	if(Value != 0.0f && bCanMove)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyProjectCPPCharacter::MoveRight(float Value)
{
	if(Value != 0.0f && bCanMove)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyProjectCPPCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMyProjectCPPCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}