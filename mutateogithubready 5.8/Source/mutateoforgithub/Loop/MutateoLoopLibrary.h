// Blueprint helper nodes for the gameplay loop (they show up under "Mutateo" in the node menu).

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "mutateoforgithub/Items/ItemDefinition.h"
#include "MutateoLoopLibrary.generated.h"

class AMutateoLoopDirector;
class UMutateoGameSubsystem;
class UMaterialInterface;

UCLASS()
class MUTATEOFORGITHUB_API UMutateoLoopLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Mutateo|Loop", meta = (WorldContext = "WorldContextObject"))
	static AMutateoLoopDirector* GetLoopDirector(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "Mutateo|Loop", meta = (WorldContext = "WorldContextObject"))
	static UMutateoGameSubsystem* GetMutateoSubsystem(const UObject* WorldContextObject);

	/** Optional: makes the loop re-check progress right now instead of on its next 0.25s poll. */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Loop", meta = (WorldContext = "WorldContextObject"))
	static void RefreshGameLoop(const UObject* WorldContextObject);

	// ---- Inventory bridge ----

	/** Finds the object implementing Mutateo Inventory on this actor, its components, or its pawn/controller. */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Inventory")
	static UObject* FindInventory(AActor* Owner);

	UFUNCTION(BlueprintCallable, Category = "Mutateo|Inventory")
	static int32 GetInventoryItemCount(AActor* Owner, TSubclassOf<UItemDefinition> Item);

	UFUNCTION(BlueprintCallable, Category = "Mutateo|Inventory")
	static bool ConsumeInventoryItems(AActor* Owner, TSubclassOf<UItemDefinition> Item, int32 Count);

	// ---- Visuals ----

	/** Puts an overlay material on (or removes it from) every mesh on the actor, and toggles custom depth for outline effects. */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Highlight")
	static void SetActorHighlighted(AActor* Actor, bool bHighlighted, UMaterialInterface* OverlayMaterial);

	// ---- Upgrades / combat ----

	UFUNCTION(BlueprintPure, Category = "Mutateo|Upgrades", meta = (WorldContext = "WorldContextObject"))
	static float GetUpgradeValue(const UObject* WorldContextObject, FName UpgradeId, float Fallback = 1.f);

	/**
	 * Fires a hitscan shot from the shooter's camera. Damage = BaseDamage x WeaponDamage upgrade.
	 * AimAssistRadius > 0 uses a sphere sweep so it's easier to hit things.
	 * Returns true if something was hit; OutHit has the impact point for your water VFX.
	 */
	UFUNCTION(BlueprintCallable, Category = "Mutateo|Combat", meta = (DefaultToSelf = "Shooter"))
	static bool FireWeapon(APawn* Shooter, FHitResult& OutHit, float BaseDamage = 10.f, float Range = 3000.f, float AimAssistRadius = 15.f);
};
