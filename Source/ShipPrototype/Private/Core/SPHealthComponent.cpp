// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SPHealthComponent.h"

#include "Core/SPGameModeBase.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogHealthComponent, Log, All);

// Sets default values for this component's properties
USPHealthComponent::USPHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MaxHealth = 100;
	DefaultHealth = 100;

	MaxArmor = 200;
	DefaultArmor = 50;
	bHasArmor = true;
	
	bIsDead = false;
	// ...
}


// Called when the game starts
void USPHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if(AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &USPHealthComponent::HandleTakeAnyDamage);
		CurrentHealth = DefaultHealth;
		if(bHasArmor) CurrentArmor = DefaultArmor;
		
	}
	
}

void USPHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if(Damage <= 0 || bIsDead)
	{
		return;
	}

	if(DamageCauser == DamagedActor)
	{
		return;
	}

	if(IsFriendly(DamagedActor, DamageCauser))
	{
		return;
	}

	AController* DamagedController = nullptr;
	if(const APawn* DamagedPawn = Cast<APawn>(GetOwner()))
	{
		DamagedController = DamagedPawn->GetController();
	}

	if(CurrentArmor <= 0.0f || !bHasArmor)
	{
		// no armor, apply damage to health
		
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
		UE_LOG(LogHealthComponent, Log, TEXT("USPHealthComponent::HandleTakeAnyDamage -> Updated health to: %s"), *FString::SanitizeFloat(CurrentHealth));
		OnHealthChanged.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);
	}
	else
	{
		// save remaining damage before applying damage to armor
		const float RemainingDamage = Damage - CurrentArmor;

		// apply damage and call armor changed event
		CurrentArmor = FMath::Clamp(CurrentArmor - Damage, 0.0f, MaxArmor);
		UE_LOG(LogHealthComponent, Log, TEXT("UPSHealthComponent::HandleTakeAnyDamage -> Updated armor to: %s"), *FString::SanitizeFloat(CurrentArmor));
		OnArmorChanged.Broadcast(this, CurrentArmor, Damage, DamageType, InstigatedBy, DamageCauser);

		// apply remaining damage to current health
		if(RemainingDamage > 0.0f)
		{
			CurrentHealth = FMath::Clamp(CurrentHealth - RemainingDamage, 0.0f, MaxHealth);
			UE_LOG(LogHealthComponent, Log, TEXT("UPSHealthComponent::HandleTakeAnyDamage -> Updated health to: %s"), *FString::SanitizeFloat(CurrentHealth));
			OnHealthChanged.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);
		}
	}

	// Damage taken, apply health regen cooldown
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_HealthRegenCooldown, this,
	                                       &USPHealthComponent::OnHealthRegenCooldown, HealthRegenCooldown, false);

	if(CurrentHealth <= 0.0f)
	{
		bIsDead = true;

		if(ASPGameModeBase* GM = Cast<ASPGameModeBase>(GetWorld()->GetAuthGameMode()))
		{
			GM->OnActorKilled.Broadcast(this, GetOwner(), DamagedController, DamageCauser, InstigatedBy);
		}
	}

}


// Called every frame
void USPHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	// try health regen
	if(bCanRegenHealth && !bIsDead)
	{
		// If health regen is in cool down return 
		if(GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_HealthRegenCooldown)) return;

		// Check if we are already at DefaultHealth or above it
		if(CurrentHealth > DefaultHealth) return;

		if(FMath::IsNearlyEqual(CurrentHealth, DefaultHealth, 0.01f))
		{
			CurrentHealth = DefaultHealth;
			return;
		}

		// Smoothly regenerate health
		CurrentHealth = UKismetMathLibrary::FInterpTo_Constant(CurrentHealth, DefaultHealth, DeltaTime, HealthRegenSpeed);
	}

}

void USPHealthComponent::OnHealthRegenCooldown()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_HealthRegenCooldown);
}

bool USPHealthComponent::IsFriendly(const AActor* ActorA, const AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr)
	{
		return false;
	}
	
	const USPHealthComponent* HealthCompA = Cast<USPHealthComponent>(ActorA->GetComponentByClass(USPHealthComponent::StaticClass()));
	const USPHealthComponent* HealthCompB = Cast<USPHealthComponent>(ActorB->GetComponentByClass(USPHealthComponent::StaticClass()));

	if (HealthCompA == nullptr || HealthCompB == nullptr)
	{
		//Assume friendly
		return true;
	}

	return HealthCompA->TeamID == HealthCompB->TeamID;
}