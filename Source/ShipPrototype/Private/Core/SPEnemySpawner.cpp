// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SPEnemySpawner.h"

#include "Core/SPEnemyBase.h"
#include "Core/SPGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
USPEnemySpawner::USPEnemySpawner()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void USPEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if(ASPGameModeBase* GM = Cast<ASPGameModeBase>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnActorKilled.AddDynamic(this, &USPEnemySpawner::OnActorKilledEvent);
	}
	

	// ...
	
}


// Called every frame
void USPEnemySpawner::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


void USPEnemySpawner::StartSpawningEnemies()
{
	StopSpawningEnemies();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_SpawnEnemy, this, &USPEnemySpawner::TrySpawnEnemy,
	                                       SpawnEnemyInterval, true, 0.0f);
}

void USPEnemySpawner::StopSpawningEnemies()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_SpawnEnemy);
}

void USPEnemySpawner::TrySpawnEnemy()
{
	if(SpawnedEnemies.Num() >= MaxEnemies || !EnemyClass) return;
	
	//find suitable spawn location
	const APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if(!Player) return;

	FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();
	RandomDirection.X = 0;
	RandomDirection.Normalize();
	//MoveLocation = TargetPlayer->GetActorLocation() + (RandomDirection * FireRange);

	FVector RandomPos = FVector
	(
		0,
		RandomDirection.Y * 1200,
		FMath::RandRange(250, 1000)
	);
	RandomPos += Player->GetActorLocation();
	RandomPos.Z = (RandomPos.Z > 0) ? RandomPos.Z : 0;

	FActorSpawnParameters SpawnParameters;
	//SpawnParameters.Instigator = OwnerPawn;
	//SpawnParameters.Owner = OwnerPawn;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	ASPEnemyBase* Enemy = GetWorld()->SpawnActor<ASPEnemyBase>(EnemyClass, RandomPos, FRotator::ZeroRotator, SpawnParameters);

	if(Enemy)
	{
		SpawnedEnemies.Add(Enemy);
	}
}

void USPEnemySpawner::OnActorKilledEvent(USPHealthComponent* InHealthComp, AActor* KilledActor,
	AController* KilledController, AActor* KillerActor, AController* KillerController)
{
	if(ASPEnemyBase* EnemyBase = Cast<ASPEnemyBase>(KilledActor))
	{
		SpawnedEnemies.Remove(EnemyBase);
	}
}


void USPEnemySpawner::ClearEnemies()
{
	for (ASPEnemyBase* Enemy : SpawnedEnemies)
	{
		if(Enemy) Enemy->Destroy();
	}
	SpawnedEnemies.Empty();
	return;
	TArray<AActor*> EnemyActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASPEnemyBase::StaticClass(), EnemyActors);
	for (AActor* EnemyActor : EnemyActors)
	{
		EnemyActor->Destroy();
	}
}
