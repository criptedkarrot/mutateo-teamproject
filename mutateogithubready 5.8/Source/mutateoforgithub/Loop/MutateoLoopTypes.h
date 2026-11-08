// Shared types for the Mutateo gameplay loop.

#pragma once

#include "CoreMinimal.h"
#include "mutateoforgithub/Items/ItemDefinition.h"
#include "MutateoLoopTypes.generated.h"

class USoundBase;

/** Names of the built-in upgrades. Use these with GetUpgradeValue / PurchaseUpgrade. */
namespace MutateoUpgradeIds
{
	inline const FName WeaponDamage(TEXT("WeaponDamage"));
	inline const FName HarvestYield(TEXT("HarvestYield"));
	inline const FName GrowthSpeed(TEXT("GrowthSpeed"));
	inline const FName MoveSpeed(TEXT("MoveSpeed"));
}

/** Every step of the vertical-slice loop, in order. */
UENUM(BlueprintType)
enum class EMutateoLoopStage : uint8
{
	NotStarted       UMETA(DisplayName = "Not Started"),
	Intro            UMETA(DisplayName = "Intro"),
	CollectSeeds     UMETA(DisplayName = "Collect Starter Crop"),
	PlantSeeds       UMETA(DisplayName = "Plant Crops"),
	FetchMaterials   UMETA(DisplayName = "Fetch Materials"),
	CraftWeapon      UMETA(DisplayName = "Craft Weapon"),
	WaitForCrops     UMETA(DisplayName = "Wait For Crops"),
	ActivateSwitch   UMETA(DisplayName = "Activate Harvest Switch"),
	DefeatMutants    UMETA(DisplayName = "Defeat Mutants"),
	DepositResources UMETA(DisplayName = "Deposit Resources"),
	VisitUpgrades    UMETA(DisplayName = "Visit Upgrade Station"),
	Replant          UMETA(DisplayName = "Replant")
};

/** One line spoken by the guide voice. Leave Text empty and Sound unset to say nothing. */
USTRUCT(BlueprintType)
struct FMutateoVoiceLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (MultiLine = true))
	FText Text;

	/** Optional voice-over audio. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
	TObjectPtr<USoundBase> Sound = nullptr;

	/** Seconds the subtitle stays up. 0 = work it out from the sound length or word count. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice", meta = (ClampMin = 0))
	float Duration = 0.f;

	FMutateoVoiceLine() {}
	explicit FMutateoVoiceLine(const FText& InText) : Text(InText) {}

	bool IsEmpty() const { return Text.IsEmpty() && Sound == nullptr; }
};

/** "Need Count of Item" — used by the crafting station recipe. */
USTRUCT(BlueprintType)
struct FMutateoItemRequirement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<UItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 1))
	int32 Count = 1;
};

/** An item the deposit station buys. */
USTRUCT(BlueprintType)
struct FMutateoSellableItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TSubclassOf<UItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0))
	int32 CoinValue = 5;

	/** This many are left in the player's inventory so they can replant. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0))
	int32 KeepForReplanting = 1;
};

/** One purchasable upgrade. Value at a level = BaseValue + ValuePerLevel * Level. */
USTRUCT(BlueprintType)
struct FMutateoUpgradeDef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade", meta = (ClampMin = 0))
	int32 BaseCost = 10;

	/** Each level costs this much more than the last (1.5 = +50%). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade", meta = (ClampMin = 1))
	float CostGrowth = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade", meta = (ClampMin = 1))
	int32 MaxLevel = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	float BaseValue = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	float ValuePerLevel = 0.25f;

	FMutateoUpgradeDef() {}
	FMutateoUpgradeDef(FName InId, const FText& InName, const FText& InDesc, int32 InCost, float InGrowth,
	                   int32 InMax, float InBase, float InPerLevel)
		: Id(InId), DisplayName(InName), Description(InDesc), BaseCost(InCost), CostGrowth(InGrowth),
		  MaxLevel(InMax), BaseValue(InBase), ValuePerLevel(InPerLevel) {}
};
