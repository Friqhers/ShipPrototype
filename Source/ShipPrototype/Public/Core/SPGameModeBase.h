// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SPGameModeBase.generated.h"

class ASPShip;
class USPEnemySpawner;
class USPHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnActorKilledSignature, USPHealthComponent*, InHealthComp, AActor*, KilledActor,
	AController*, KilledController,	AActor*, KillerActor, AController*, KillerController);

/**
 * 
 */
UCLASS()
class SHIPPROTOTYPE_API ASPGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	ASPGameModeBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USPEnemySpawner* EnemySpawner;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	float RespawnTime;
public:
	UPROPERTY(BlueprintReadOnly)
	int NumberOfWins=0;

	UPROPERTY(BlueprintReadOnly)
	int NumberOfLosses=0;
public:
	//EVENTS
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnActorKilledSignature OnActorKilled;
protected:
	virtual void BeginPlay() override;
public:

	/**
	 * @brief This function is called when any actor is killed in the game.
	 * 
	 * @param InHealthComp The health component of the killed actor, which contains health-related information.
	 * @param KilledActor The actor that has been killed.
	 * @param KilledController The controller associated with the killed actor.
	 * @param KillerActor The actor that dealt the killing blow.
	 * @param KillerController The controller associated with the killer actor.
	 */
	UFUNCTION()
	virtual void OnActorKilledEvent(USPHealthComponent* InHealthComp, AActor* KilledActor, AController* KilledController, AActor* KillerActor, AController* KillerController);

	/**
	 * @brief This function is called when the player either reaches the end of the game or is killed.
	 *
	 * This function clears all enemies from the level and respawns the player by calling .
	 */
	virtual void RestartGame();
	
	//Respawns the player 0 at player start.
	virtual void RespawnPlayer();
	
	virtual void OnPlayerReachesEndZone(ASPShip* PlayerShip);
};
