// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PickUp.h"
#include "BaseWeapon.h"
#include "MyProjectCPPCharacter.generated.h"


class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;

UCLASS(config=Game)
class AMyProjectCPPCharacter : public ACharacter
{
	GENERATED_BODY()

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere)
		class USceneComponent* HoldingComponent;

public:
	AMyProjectCPPCharacter();

	UFUNCTION(BlueprintCallable, Category = "Damage")
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
protected:
	virtual void BeginPlay();
	virtual void Tick(float DeltaSeconds) override;

//private:
//	void updateHealthAndArmor();
public:

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(BlueprintReadWrite, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* FP_MuzzleLocation;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(BlueprintReadWrite, Category=Projectile)
	TSubclassOf<class AMyProjectCPPProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	UPROPERTY(EditAnywhere)
	class APickUp* CurrentItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float health;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float armor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float maxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float maxArmor;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float interpolatedHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float interpolatedArmor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float newHealthPercent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	float newArmorPercent;

	bool bCanMove;
	bool bHoldingItem;
	bool bInspecting;

	float PitchMin;
	float PitchMax;

	FVector HoldingComp;
	FRotator LastRotation;

	FVector Start;
	FVector ForwardVector;
	FVector End;

	FHitResult Hit;

	FComponentQueryParams DefaultComponentQueryParams;

	FCollisionResponseParams DefaultResponseParams;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	TArray<ABaseWeapon*> weapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	int currentWeapon;

protected:
	
	DECLARE_DELEGATE_OneParam(weaponIndex, int32);

	void SwitchToWeapon(int index);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void SwitchWeaponMesh(int index);

	/** Fires a projectile. */
	void OnFire();

	void OnAction();

	void OnInspect();

	void OnInspectReleased();

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;

	void ToggleMovement();
	void ToggleItemPickup();


protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

