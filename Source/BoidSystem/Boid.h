// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Boid.generated.h"

UCLASS()
class BOIDSYSTEM_API ABoid : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABoid();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid Properties")
	float Speed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boid Properties")
	float MaxSpeed = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Parameters")
	float BoundryRadius = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Parameters")
	float BoundryStrength = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Boid Parameters")
	FVector BoundryCenter = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Avoidance")
	float AvoidanceDistance = 800.0f;
	UPROPERTY(EditAnywhere, Category = "Avoidance")
	float AvoidanceStrength = 1500.0f;

	FVector CalculateAvoidanceForce(FHitResult& Hit);

	FVector CalculateBoundaryForce();

	FVector Velocity;

private:
	UPROPERTY()
	TArray<ABoid*> AllBoidsCache;

public:
	void SetAllBoidsReference(const TArray<ABoid*>& AllBoids);

	void GetNeighborActors(TArray<AActor*>& OutActors) const;
};
