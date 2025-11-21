// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoidSpawner.generated.h"

class ABoid;
struct FBOidOctreeElement;
struct FBoidOctreeNode;

struct FBoidOctreeElement
{
	FVector Location;
	class ABoid* BoidPtr;
};

struct FBoidOctreeNode
{
	FBox Bounds;
	TArray<FBoidOctreeElement> Elements;
	TUniquePtr<FBoidOctreeNode> Children[8];

	FBoidOctreeNode(const FBox& InBounds) : Bounds(InBounds) {}
};

class FBoidOctreeManager
{
public:
	FBox WorldBounds;

	void Initialize(const FVector& Center, float Extent);

	void Reset() { RootNode = nullptr; }

	void AddBoid(ABoid* Boid);

	void QueryNeighbors(const FVector& Location, float Radius, TArray<ABoid*>& OutNeighbors);

private:
	TUniquePtr<FBoidOctreeNode> RootNode;
};

UCLASS()
class BOIDSYSTEM_API ABoidSpawner : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABoidSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Boid Spawning")
	TSubclassOf<class ABoid> BoidClass;
	UPROPERTY(EditAnywhere, Category = "Boid Spawning", meta = (ClampMin = "1"))

	int32 NumberOfBoids = 10;

	FBoidOctreeManager OctreeManager;

private:
	UPROPERTY()
	TArray<ABoid*> SpawnedBoids;

};
