// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SPEnemySpawner.generated.h"


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
	UPROPERTY()
	TArray<ASPEnemyBase*> SpawnedEnemies;

	FTimerHandle TimerHandle_SpawnEnemy;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void ClearEnemies();
	
	void StartSpawningEnemies();
	void StopSpawningEnemies();

	UFUNCTION()
	virtual void OnActorKilledEvent(USPHealthComponent* InHealthComp, AActor* KilledActor, AController* KilledController, AActor* KillerActor, AController* KillerController);

private:
	void TrySpawnEnemy();
};