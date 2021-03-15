// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "NiagaraCommon.h"
#include "GameFramework/Actor.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "BaseWeapon.generated.h"

UCLASS()
class MYPROJECTCPP_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	ABaseWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class UNiagaraSystem* ParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class UAnimMontage* FiringAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class USoundBase* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	class USoundBase* FiringSound;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Projectile)
	TSubclassOf<class AMyProjectCPPProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	int maxAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	int ammo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapon)
	float fireRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	bool hitscan;

	UPROPERTY(BlueprintReadOnly)
	FVector lineTraceStartPoint;

	UPROPERTY(BlueprintReadOnly)
	FVector lineTraceEndPoint;

	UPROPERTY(BlueprintReadOnly)
	FVector lineTraceHitLocation;

	UPROPERTY(BlueprintReadOnly)
	FVector impactNormal;

	UFUNCTION(BlueprintCallable)
    bool FireWeapon();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	int damage;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
    bool GetReadyToFire() const { return readyToFire; }

	UFUNCTION(BlueprintCallable)
    void ToggleReadyToFire() { readyToFire = !readyToFire; }

	UFUNCTION(BlueprintCallable)
	void setActive(int index);	
private:

	FTimerHandle handle;
	bool readyToFire;
	float lastBulletFired;
};
