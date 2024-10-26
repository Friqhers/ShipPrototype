// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SPEnemySpawner.generated.h"


class USPHealthComponent;
class ASPEnemyBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHIPPROTOTYPE_API USPEnemySpawner : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USPEnemySpawner();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category= "Spawner|Settings")
	TSubclassOf<ASPEnemyBase> EnemyClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawner|Settings")
	float SpawnEnemyInterval = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spawner|Settings")
	int MaxEnemies = 10.f;
private:
	// Stores all currently spawned and alive enemies. Enemies are removed from this list 
	// by the OnActorKilledEvent method when they are killed.
	UPROPERTY()
	TArray<ASPEnemyBase*> SpawnedEnemies;

	FTimerHandle TimerHandle_SpawnEnemy;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * @brief Destroys all the enemies in SpawnedEnemies, and clear the array 
	 */
	virtual void ClearEnemies();

	// Starts the spawning by setting a timer(TimerHandle_SpawnEnemy) that calls TrySpawnEnemy
	void StartSpawningEnemies();

	// Stops the spawning by clearing the timer(TimerHandle_SpawnEnemy)
	void StopSpawningEnemies();

	/**
	 * @brief Called whenever an actor killed. This method checks if the death actor is an ASPEnemyBase,
	 *  and if it is the actor is removed from the SpawnedEnemies list.
	 *  
	 * @param InHealthComp The health component of the killed actor, which contains health-related information.
	 * @param KilledActor The actor that has been killed.
	 * @param KilledController The controller associated with the killed actor.
	 * @param KillerActor The actor that dealt the killing blow.
	 * @param KillerController The controller associated with the killer actor.
	 */
	UFUNCTION()
	virtual void OnActorKilledEvent(USPHealthComponent* InHealthComp, AActor* KilledActor, AController* KilledController, AActor* KillerActor, AController* KillerController);

private:
	/**
	 * @brief Attempts to spawn an enemy somewhere that is not noticeable to the player. The enemy randomly
	 * positioned either far up-left or up-right of the player
	 */
	void TrySpawnEnemy();
};