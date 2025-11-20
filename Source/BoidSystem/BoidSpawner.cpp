// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidSpawner.h"
#include "Boid.h"

// Sets default values
ABoidSpawner::ABoidSpawner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABoidSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (BoidClass)
	{
		for (int i = 0; i < NumberOfBoids; i++)
		{

			FVector SpawnOffset = FVector(FMath::RandRange(-500, 500),
				FMath::RandRange(-500, 500),
				FMath::RandRange(-500, 500));

			FVector SpawnLocation = GetActorLocation() + SpawnOffset;
			FRotator SpawnRotation = FRotator::ZeroRotator;

			ABoid* NewBoid = GetWorld()->SpawnActor<ABoid>(BoidClass, SpawnLocation, SpawnRotation);

			if (NewBoid)
			{
				SpawnedBoids.Add(NewBoid);
			}

			for (ABoid* Boid : SpawnedBoids)
			{
				if (Boid)
				{
					Boid->SetAllBoidsReference(SpawnedBoids);
				}
			}
		}
	}

}

// Called every frame
void ABoidSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

