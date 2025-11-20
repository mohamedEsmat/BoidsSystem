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
		FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 100);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		GetWorld()->SpawnActor<AActor>(BoidClass, SpawnLocation, SpawnRotation);
	}

}

// Called every frame
void ABoidSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

