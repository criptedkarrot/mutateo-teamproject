// A simple melee enemy: chases the player, attacks in range, dies and drops resources.
// Make a Blueprint child and give it a skeletal mesh + anim blueprint (e.g. the Manny mannequin, tinted green).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MutateoMutant.generated.h"

UCLASS(Blueprintable)
class MUTATEOFORGITHUB_API AMutateoMutant : public ACharacter
{
	GENERATED_BODY()

public:
	AMutateoMutant();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Mutant", meta = (ClampMin = 1))
	float MaxHealth = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Mutant", meta = (ClampMin = 0))
	float AttackDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Mutant", meta = (ClampMin = 0))
	float AttackRange = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Mutant", meta = (ClampMin = 0.1))
	float AttackCooldown = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Mutant", meta = (ClampMin = 0))
	float ChaseSpeed = 350.f;

	/** Waits this long after spawning before it starts chasing (time for a "rise up" animation). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Mutant", meta = (ClampMin = 0))
	float WakeUpDelay = 1.f;

	/** Pickup spawned on death, e.g. a child of bp_pickup_item set to item_carrot. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Drops")
	TSubclassOf<AActor> DropClass;

	/** Drops this many when the Harvest Yield upgrade isn't registered; otherwise uses the upgrade's value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Drops", meta = (ClampMin = 0))
	int32 BaseDropCount = 5;

	/** How long the body stays after dying. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mutateo|Mutant", meta = (ClampMin = 0))
	float CorpseLifeSpan = 2.f;

	UFUNCTION(BlueprintPure, Category = "Mutateo|Mutant")
	bool IsDead() const { return bDead; }

	UFUNCTION(BlueprintPure, Category = "Mutateo|Mutant")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Mutateo|Mutant")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Mutateo|Mutant")
	void OnDamaged(float Damage, float HealthRemaining);

	/** Play your attack montage here. Damage has already been applied to the target. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mutateo|Mutant")
	void OnAttack(AActor* Target);

	/** Play a death effect / ragdoll here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mutateo|Mutant")
	void OnDeath();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	void Die();
	void SpawnDrops();

	float Health = 0.f;
	float TimeSinceAttack = 0.f;
	float TimeAlive = 0.f;
	bool bDead = false;
	bool bUseDirectChase = false;
};
