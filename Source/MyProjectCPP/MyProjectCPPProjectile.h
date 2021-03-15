// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyProjectCPPProjectile.generated.h"

//class USphereComponent;
class UBoxComponent;
class UProjectileMovementComponent;

UCLASS(config=Game)
class AMyProjectCPPProjectile : public AActor
{
	GENERATED_BODY()

	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	UBoxComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, Category = Damage)
	int Damage;

public:
	AMyProjectCPPProjectile();

	void Initialize(int Damage);
	/** called when projectile hits something */
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	UFUNCTION(BlueprintCallable)
	UBoxComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UFUNCTION(BlueprintCallable)
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UFUNCTION(BlueprintCallable)
    int GetDamage(){ return Damage; }

	UFUNCTION(BlueprintCallable)
    void SetDamage(int DamageToSet){ this->Damage = DamageToSet; }
};

