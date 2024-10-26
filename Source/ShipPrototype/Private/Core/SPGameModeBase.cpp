// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SPGameModeBase.h"

#include "Core/SPEnemyBase.h"
#include "Core/SPEnemySpawner.h"
#include "Core/SPShip.h"
#include "Kismet/GameplayStatics.h"

ASPGameModeBase::ASPGameModeBase()
{
	EnemySpawner = CreateDefaultSubobject<USPEnemySpawner>(TEXT("EnemySpawner"));
	
	OnActorKilled.AddDynamic(this, &ASPGameModeBase::OnActorKilledEvent);
	RespawnTime = 2.0f;
}


void ASPGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	if(EnemySpawner)
	{
		EnemySpawner->StartSpawningEnemies();
	}
}

void ASPGameModeBase::OnActorKilledEvent(USPHealthComponent* InHealthComp, AActor* KilledActor,
                                         AController* KilledController, AActor* KillerActor, AController* KillerController)
{
	if(APlayerController* PC = Cast<APlayerController>(KilledController))
	{
		//player killed
		NumberOfLosses++;
		
		// restart the game
		FTimerHandle TimerHandle_RestartGame;
		//const FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ASPGameModeBase::RespawnPlayer, PC);
		GetWorldTimerManager().SetTimer(TimerHandle_RestartGame, this, &ASPGameModeBase::RestartGame
		                                , RespawnTime, false);
	}
}

void ASPGameModeBase::RestartGame()
{
	if(EnemySpawner)
	{
		EnemySpawner->StopSpawningEnemies();
	}
	
	EnemySpawner->ClearEnemies();
	RespawnPlayer();

	if(EnemySpawner)
	{
		EnemySpawner->StartSpawningEnemies();
	}
}


void ASPGameModeBase::RespawnPlayer()
{
	if(APlayerController* PC =UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		RestartPlayer(PC);
	}
}

void ASPGameModeBase::OnPlayerReachesEndZone(ASPShip* PlayerShip)
{
	if(!PlayerShip) return;
	
	// destroy player ship
	PlayerShip->DetachFromControllerPendingDestroy();
	PlayerShip->Destroy();
	
	NumberOfWins++;
	
	// restart the game after specific time
	FTimerHandle TimerHandle_RestartGame;
	GetWorldTimerManager().SetTimer(TimerHandle_RestartGame, this, &ASPGameModeBase::RestartGame
										, RespawnTime, false);
}

