// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"
#include "MyProjectCPPProjectile.h"
#include <activation.h>


#include "MyProjectCPPCharacter.h"
#include "Engine/World.h"
#include "Input/Events.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ammo = 25;
	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(0.0f, 0.0f, -35.0f);

	// weapon is reloaded
	readyToFire = true;
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if weapon is not ready, check wether it's ready again
	if (!readyToFire)
	{
		if (GetWorld()->TimeSeconds > (lastBulletFired + (1.0f / fireRate)))
		//if (GetWorldTimerManager().GetTimerElapsed(handle) > fireRate)
		{
			UE_LOG(LogTemp, Warning, TEXT("Flipping ReadyToFire!"));
			readyToFire = true;
			//GetWorldTimerManager().SetTimer(handle, 2 * fireRate, true, 0.0f);	
		}
	}
}

void ABaseWeapon::setActive(int index)
{
	// start fireRate timer
	//GetWorldTimerManager().SetTimer(handle, 2 * fireRate, true, 0.0f);	
}

bool ABaseWeapon::FireWeapon()
{
	auto player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	auto playerCharacter = Cast<AMyProjectCPPCharacter>(player);
	
	if (hitscan)
	{
		lineTraceHitLocation = FVector(0.0f, 0.0f, 0.0f);
		USceneComponent* muzzleLocation = playerCharacter->FP_MuzzleLocation;
		lineTraceStartPoint = muzzleLocation->GetComponentLocation();
		FVector cameraForwardVector = playerCharacter->GetFirstPersonCameraComponent()->GetForwardVector();
		cameraForwardVector *= 5000;
		lineTraceEndPoint = lineTraceStartPoint + cameraForwardVector;

		FHitResult Outhit;
		FCollisionQueryParams CollisionParams;
		bool bHasHit = GetWorld()->LineTraceSingleByChannel(Outhit, lineTraceStartPoint, lineTraceEndPoint,
		                                                    ECC_Visibility, CollisionParams);

		if (bHasHit)
		{
			FHitResult result;
			Outhit.Actor->ReceiveHit(Outhit.GetComponent(), nullptr, nullptr, 0,
			                         FVector(0, 0, 0), FVector(0, 0, 0), FVector(0, 0, 0),
			                         result);
			lineTraceHitLocation = Outhit.Location;
		}
	}
	else
	{
		UWorld* const World = GetWorld();
		const FRotator SpawnRotation = playerCharacter->GetControlRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		//		const FVector SpawnLocation = ((playerCharacter->FP_MuzzleLocation != nullptr) ? playerCharacter->FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

		const FVector SpawnLocation = playerCharacter->GetFirstPersonCameraComponent()->GetComponentLocation() +
			GunOffset;
		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		if (ProjectileClass != nullptr)
		{
			World->SpawnActor<AMyProjectCPPProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		}
	}
	ammo--;
	// weapon can't fire anymore, wait for tick() to update readyToFire to "true"
	readyToFire = false;
	lastBulletFired = GetWorld()->TimeSeconds;
	return true;
}
