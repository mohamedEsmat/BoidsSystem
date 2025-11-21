// Fill out your copyright notice in the Description page of Project Settings.


#include "Boid.h"
#include <Kismet/GameplayStatics.h>
#include "Kismet/KismetSystemLibrary.h" 
#include "DrawDebugHelpers.h" 
#include "BoidSpawner.h"

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

		TArray<ABoid*> BoidNeighbors;
		float MaxRadius = FMath::Max(FMath::Max(SeparationDistance, AlignmentDistance), CohesionDistance);

        QueryNeighborsFromOctree(MaxRadius, BoidNeighbors); 

		FVector SeparationForce = CalculateSeparationForce(BoidNeighbors);
		FVector AlignmentForce = CalculateAlignmentForce(BoidNeighbors);
		FVector CohesionForce = CalculateCohesionForce(BoidNeighbors);

		Acceleration += SeparationForce + AlignmentForce + CohesionForce;
	}
	Velocity += Acceleration * DeltaTime;

	float DampingFactor = 0.95f;
	Velocity *= DampingFactor;

	if (!Velocity.IsNearlyZero())
	{
		if (Velocity.SizeSquared() > (MaxSpeed * MaxSpeed))
		{
			Velocity = Velocity.GetClampedToSize(0.0f, MaxSpeed);
		}
		else
		{
			Velocity = Velocity.GetSafeNormal() * MaxSpeed;
		}
		SetActorRotation(Velocity.Rotation());
	}

	FVector NewLocation = GetActorLocation() + (Velocity * DeltaTime);
	SetActorLocation(NewLocation);
}

void ABoid::SetAllBoidsReference(const TArray<ABoid*>& AllBoids)
{
	AllBoidsCache = AllBoids;
}

void ABoid::SetParentSpawner(ABoidSpawner* Spawner)
{
	ParentSpawner = Spawner;
}

void ABoid::QueryNeighborsFromOctree(float Radius, TArray<ABoid*>& OutNeighbors) const
{
	if (ParentSpawner)
	{
		ParentSpawner->OctreeManager.QueryNeighbors(GetActorLocation(), Radius, OutNeighbors);
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

FVector ABoid::CalculateSeparationForce(const TArray<ABoid*>& Neighbours)
{
	FVector SeparationVector = FVector::ZeroVector;
	int BoidsToSeparateFrom = 0;
	FVector CurrentLocation = GetActorLocation();

	for (ABoid* NeighborBoid : Neighbours)
	{
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

FVector ABoid::CalculateAlignmentForce(const TArray<ABoid*>& Neighbours)
{
	FVector AverageVelocity = FVector::ZeroVector;
	int NeighborCount = 0;
	FVector CurrentLocation = GetActorLocation();

	for (ABoid* NeighborBoid : Neighbours)
	{
		if (!NeighborBoid || NeighborBoid == this)
		{
			continue;
		}

		float Distance = FVector::Dist(CurrentLocation, NeighborBoid->GetActorLocation());
		if (Distance > 0.0f && Distance < AlignmentDistance)
		{
			AverageVelocity += NeighborBoid->Velocity;
			NeighborCount++;
		}
	}
	if (NeighborCount > 0)
	{
		AverageVelocity /= NeighborCount;
		FVector AlignmentSteering = (AverageVelocity - Velocity);
		return AlignmentSteering * AlignmentStrength;
	}
	return FVector::ZeroVector;
}

FVector ABoid::CalculateCohesionForce(const TArray<ABoid*>& Neighbours)
{
	FVector CenterOfMass = FVector::ZeroVector;
	int NeighborCount = 0;
	FVector CurrentLocation = GetActorLocation();
	for (ABoid* NeighborBoid : Neighbours)
	{
		if (!NeighborBoid || NeighborBoid == this)
		{
			continue;
		}
		float Distance = FVector::Dist(CurrentLocation, NeighborBoid->GetActorLocation());
		if (Distance > 0.0f && Distance < CohesionDistance)
		{
			CenterOfMass += NeighborBoid->GetActorLocation();
			NeighborCount++;
		}
	}
	if (NeighborCount > 0)
	{
		CenterOfMass /= NeighborCount;
		FVector DesiredDirection = CenterOfMass - CurrentLocation;
		return DesiredDirection.GetSafeNormal() * CohesionStrength;
	}
	return FVector::ZeroVector;
}

