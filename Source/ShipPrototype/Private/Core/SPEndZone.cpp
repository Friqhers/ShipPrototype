// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SPEndZone.h"
#include "Components/BoxComponent.h"
#include "Core/SPGameModeBase.h"
#include "Core/SPShip.h"

// Sets default values
ASPEndZone::ASPEndZone()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(RootComponent);

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ASPEndZone::OnBoxBeginOverlap);
}

// Called when the game starts or when spawned
void ASPEndZone::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASPEndZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASPEndZone::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(ASPShip* PlayerShip = Cast<ASPShip>(OtherActor))
	{
		if(ASPGameModeBase* SPGameMode =Cast<ASPGameModeBase>(GetWorld()->GetAuthGameMode()))
		{
			SPGameMode->OnPlayerReachesEndZone(PlayerShip);
		}
	}
}

