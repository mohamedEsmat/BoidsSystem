// Fill out your copyright notice in the Description page of Project Settings.


#include "Boid.h"
#include <Kismet/GameplayStatics.h>
#include "Kismet/KismetSystemLibrary.h" 
#include "DrawDebugHelpers.h" 

// Sets default values
ABoid::ABoid()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

}

// Called when the game starts or when spawned
void ABoid::BeginPlay()
{
	Super::BeginPlay();

	Velocity = FMath::VRand() * Speed;

}

// Called every frame
void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Acceleration = FVector::ZeroVector;
	FHitResult AvoidanceHit;
	FVector AvoidanceForce = CalculateAvoidanceForce(AvoidanceHit);
	if (!AvoidanceForce.IsNearlyZero())
	{
		Acceleration += AvoidanceForce;
	}
	else
	{
		FVector BoundaryForce = CalculateBoundaryForce();
		Acceleration += BoundaryForce;

		TArray<AActor*> Neighbors;
		GetNeighborActors(Neighbors);

		FVector SeparationForce = CalculateSeparationForce(Neighbors);
		Acceleration += SeparationForce;
	}
	Velocity += Acceleration * DeltaTime;

	if (!Velocity.IsNearlyZero())
	{
		Velocity = Velocity.GetClampedToSize(0.0f, MaxSpeed);
		SetActorRotation(Velocity.Rotation());
	}

	FVector NewLocation = GetActorLocation() + (Velocity * DeltaTime);
	SetActorLocation(NewLocation);
}

void ABoid::SetAllBoidsReference(const TArray<ABoid*>& AllBoids)
{
	AllBoidsCache = AllBoids;
}

void ABoid::GetNeighborActors(TArray<AActor*>& OutActors) const
{
	for (ABoid* Boid : AllBoidsCache)
	{
		if (Boid)
		{
			OutActors.Add(Boid);
		}
	}
}

FVector ABoid::CalculateBoundaryForce()
{
	FVector CurrentLocation = GetActorLocation();
	FVector CenterToBoid = CurrentLocation - BoundryCenter;
	float Distance = CenterToBoid.Length();
	if (Distance > BoundryRadius)
	{
		FVector SteeringVector = BoundryCenter - CurrentLocation;
		SteeringVector.Normalize();

		float ExceedDistance = Distance - BoundryRadius;
		return SteeringVector * ExceedDistance * BoundryStrength;
	}
	return FVector::ZeroVector;
}

FVector ABoid::CalculateAvoidanceForce(FHitResult& Hit)
{
	FVector StartLocation = GetActorLocation();
	FVector TraceDirection = Velocity.GetSafeNormal();
	FVector EndLocation = StartLocation + (TraceDirection * AvoidanceDistance);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	bool bHit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		StartLocation,
		EndLocation,
		UEngineTypes::ConvertToTraceType(ECC_Visibility),
		true,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		Hit,
		true
	);

	if (bHit)
	{
		FVector SteeringVector = Hit.Normal;
		SteeringVector.Normalize();
		float ReactionFactor = 1.0f - (Hit.Distance / AvoidanceDistance);
		return SteeringVector * AvoidanceStrength * ReactionFactor;
	}
	return FVector::ZeroVector;
}

FVector ABoid::CalculateSeparationForce(const TArray<AActor*>& Neighbours)
{
	FVector SeparationVector = FVector::ZeroVector;
	int BoidsToSeparateFrom = 0;
	FVector CurrentLocation = GetActorLocation();

	for (AActor* NeighborActor : Neighbours)
	{
		ABoid* NeighborBoid = Cast<ABoid>(NeighborActor);
		if (!NeighborBoid || NeighborBoid == this)
		{
			continue;
		}

		FVector NeighborLocation = NeighborBoid->GetActorLocation();
		float Distance = FVector::Dist(CurrentLocation, NeighborLocation);

		if (Distance > 0.0f && Distance < SeparationDistance)
		{
			FVector AwayVector = CurrentLocation - NeighborLocation;
			SeparationVector += AwayVector.GetSafeNormal() / Distance;
			BoidsToSeparateFrom++;
		}
	}
	if (BoidsToSeparateFrom > 0)
	{
		SeparationVector = (SeparationVector / BoidsToSeparateFrom) * SeparationStrength;
	}
	return SeparationVector;
}

