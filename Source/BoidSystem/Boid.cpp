// Fill out your copyright notice in the Description page of Project Settings.


#include "Boid.h"
#include <Kismet/GameplayStatics.h>

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

	FVector BoundaryForce = CalculateBoundaryForce();
	Velocity += BoundaryForce * DeltaTime;

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

