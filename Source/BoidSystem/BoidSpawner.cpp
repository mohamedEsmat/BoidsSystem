// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidSpawner.h"
#include "Boid.h"

void FBoidOctreeManager::Initialize(const FVector& Center, float Extent)
{
	FVector HalfExntent(Extent, Extent, Extent);
	WorldBounds = FBox(Center - HalfExntent, Center + FVector(Extent));
	RootNode = MakeUnique<FBoidOctreeNode>(WorldBounds);
}

void FBoidOctreeManager::AddBoid(ABoid* Boid)
{
	if (RootNode && Boid)
	{
		FBoidOctreeElement NewElement;
		NewElement.BoidPtr = Boid;
		NewElement.Location = Boid->GetActorLocation();
		RootNode->AddElement(NewElement);
	}
}

void RecursiveQuery(const FBoidOctreeNode* Node, const FVector& Location, float RadiusSq, TArray<ABoid*>& OutNeighbors)
{
	if (!Node)
	{
		return;
	}

	for (const FBoidOctreeElement& Element : Node->Elements)
	{
		if (FVector::DistSquared(Element.Location, Location) <= RadiusSq)
		{
			OutNeighbors.Add(Element.BoidPtr);
		}
	}

	if (Node->Children[0])
	{
		for (int i = 0; i < 8; i++)
		{
			if (Node->Children[i]->Bounds.ComputeSquaredDistanceToPoint(Location) <= RadiusSq)
			{
				RecursiveQuery(Node->Children[i].Get(), Location, RadiusSq, OutNeighbors);
			}
		}
	}
}

void FBoidOctreeManager::QueryNeighbors(const FVector& Location, float Radius, TArray<ABoid*>& OutNeighbors)
{
	if (RootNode)
	{
		RecursiveQuery(RootNode.Get(), Location, Radius * Radius, OutNeighbors);
	}
}

int FBoidOctreeNode::GetChildIndex(const FVector& Location) const
{
	FVector Center = Bounds.GetCenter();
	int Index = 0;
	if (Location.X >= Center.X) Index |= 4;
	if (Location.Y >= Center.Y) Index |= 2;
	if (Location.Z >= Center.Z) Index |= 1;
	return Index;
}

void FBoidOctreeNode::Subdivide()
{
	FVector Center = Bounds.GetCenter();
	FVector HalfExtent = Bounds.GetExtent() / 2.0f;

	for (int i = 0; i < 8; i++)
	{
		FVector ChildCenter = Center;
		if (i & 4) ChildCenter.X += HalfExtent.X; else ChildCenter.X = Center.X - HalfExtent.X;
		if (i & 2) ChildCenter.Y += HalfExtent.Y; else ChildCenter.Y = Center.Y - HalfExtent.Y;
		if (i & 1) ChildCenter.Z += HalfExtent.Z; else ChildCenter.Z = Center.Z - HalfExtent.Z;

		FBox ChildBounds(ChildCenter - HalfExtent, ChildCenter + HalfExtent);
		Children[i] = MakeUnique<FBoidOctreeNode>(ChildBounds);
	}

	for (const FBoidOctreeElement& Element : Elements)
	{
		int ChildIndex = GetChildIndex(Element.Location);
		Children[ChildIndex]->AddElement(Element);
	}

	Elements.Empty();
}

void FBoidOctreeNode::AddElement(const FBoidOctreeElement& Element)
{
	if (Children[0])
	{
		int ChildIndex = GetChildIndex(Element.Location);
		Children[ChildIndex]->AddElement(Element);
		return;
	}

	if (Elements.Num() >= MAX_ELEMENTS && Bounds.GetExtent().GetMax() > MIN_NODE_SIZE)
	{
		Subdivide();
		AddElement(Element);
		return;
	}
	Elements.Add(Element);
}

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
		}

		if (SpawnedBoids.Num() > 0)
		{
			OctreeManager.Initialize(GetActorLocation(), 5000.0f);
		}

		for (ABoid* Boid : SpawnedBoids)
		{
			if (Boid)
			{
				Boid->SetParentSpawner(this);
			}
		}
	}

}

// Called every frame
void ABoidSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	OctreeManager.Reset();
	for (ABoid* Boid : SpawnedBoids)
	{
		OctreeManager.AddBoid(Boid);
	}
}

