#include "MutateoLoopLibrary.h"

#include "MutateoGameSubsystem.h"
#include "MutateoInventoryInterface.h"
#include "MutateoLoopDirector.h"
#include "MutateoProjectBridge.h"
#include "Components/MeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

AMutateoLoopDirector* UMutateoLoopLibrary::GetLoopDirector(const UObject* WorldContextObject)
{
	const UMutateoGameSubsystem* Subsystem = GetMutateoSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetDirector() : nullptr;
}

UMutateoGameSubsystem* UMutateoLoopLibrary::GetMutateoSubsystem(const UObject* WorldContextObject)
{
	if (!GEngine || !WorldContextObject) return nullptr;
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
	return World ? World->GetSubsystem<UMutateoGameSubsystem>() : nullptr;
}

void UMutateoLoopLibrary::RefreshGameLoop(const UObject* WorldContextObject)
{
	if (AMutateoLoopDirector* Director = GetLoopDirector(WorldContextObject))
	{
		Director->EvaluateStage();
	}
}

UObject* UMutateoLoopLibrary::FindInventory(AActor* Owner)
{
	if (!Owner) return nullptr;

	auto Implements = [](const UObject* Object)
	{
		return Object && Object->GetClass()->ImplementsInterface(UMutateoInventoryInterface::StaticClass());
	};

	TArray<AActor*, TInlineAllocator<3>> Candidates;
	Candidates.Add(Owner);
	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		if (Pawn->GetController()) Candidates.Add(Pawn->GetController());
	}
	else if (const AController* Controller = Cast<AController>(Owner))
	{
		if (Controller->GetPawn()) Candidates.Add(Controller->GetPawn());
	}

	for (AActor* Candidate : Candidates)
	{
		if (Implements(Candidate)) return Candidate;
		for (UActorComponent* Component : Candidate->GetComponents())
		{
			if (Implements(Component)) return Component;
		}
	}
	return nullptr;
}

int32 UMutateoLoopLibrary::GetInventoryItemCount(AActor* Owner, TSubclassOf<UItemDefinition> Item)
{
	return MutateoBridge::CountItems(Owner, Item);
}

bool UMutateoLoopLibrary::ConsumeInventoryItems(AActor* Owner, TSubclassOf<UItemDefinition> Item, int32 Count)
{
	return MutateoBridge::ConsumeItems(Owner, Item, Count);
}

void UMutateoLoopLibrary::SetActorHighlighted(AActor* Actor, bool bHighlighted, UMaterialInterface* OverlayMaterial)
{
	if (!IsValid(Actor)) return;

	TArray<UMeshComponent*> Meshes;
	Actor->GetComponents<UMeshComponent>(Meshes);
	for (UMeshComponent* Mesh : Meshes)
	{
		if (OverlayMaterial || !bHighlighted)
		{
			Mesh->SetOverlayMaterial(bHighlighted ? OverlayMaterial : nullptr);
		}
		Mesh->SetRenderCustomDepth(bHighlighted);
		Mesh->SetCustomDepthStencilValue(bHighlighted ? 1 : 0);
	}
}

float UMutateoLoopLibrary::GetUpgradeValue(const UObject* WorldContextObject, FName UpgradeId, float Fallback)
{
	const UMutateoGameSubsystem* Subsystem = GetMutateoSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetUpgradeValue(UpgradeId, Fallback) : Fallback;
}

bool UMutateoLoopLibrary::FireWeapon(APawn* Shooter, FHitResult& OutHit, float BaseDamage, float Range, float AimAssistRadius)
{
	OutHit = FHitResult();
	if (!Shooter) return false;
	UWorld* World = Shooter->GetWorld();
	if (!World) return false;

	FVector Start;
	FRotator Rotation;
	if (AController* Controller = Shooter->GetController())
	{
		Controller->GetPlayerViewPoint(Start, Rotation);
	}
	else
	{
		Shooter->GetActorEyesViewPoint(Start, Rotation);
	}
	const FVector Direction = Rotation.Vector();
	const FVector End = Start + Direction * Range;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(MutateoFireWeapon), false, Shooter);
	TArray<AActor*> Attached;
	Shooter->GetAttachedActors(Attached);
	Params.AddIgnoredActors(Attached);

	FCollisionObjectQueryParams ObjectTypes;
	ObjectTypes.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
	ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);

	const bool bHit = AimAssistRadius > 0.f
		? World->SweepSingleByObjectType(OutHit, Start, End, FQuat::Identity, ObjectTypes, FCollisionShape::MakeSphere(AimAssistRadius), Params)
		: World->LineTraceSingleByObjectType(OutHit, Start, End, ObjectTypes, Params);

	if (!bHit)
	{
		OutHit.TraceStart = Start;
		OutHit.TraceEnd = End;
		OutHit.Location = End;
		OutHit.ImpactPoint = End;
		return false;
	}

	if (AActor* HitActor = OutHit.GetActor())
	{
		const float Multiplier = GetUpgradeValue(Shooter, MutateoUpgradeIds::WeaponDamage, 1.f);
		UGameplayStatics::ApplyPointDamage(HitActor, BaseDamage * Multiplier, Direction, OutHit,
		                                   Shooter->GetController(), Shooter, UDamageType::StaticClass());
	}
	return true;
}

// =====================================================================================
// Project bridge (see MutateoProjectBridge.h). Lives in this file so Live Coding can pick it up.
// =====================================================================================


#include "MutateoCropPlotInterface.h"
#include "MutateoInventoryInterface.h"
#include "mutateoforgithub/Items/ItemInstance.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "UObject/UnrealType.h"

namespace
{
	const FName NAME_InventoryArray(TEXT("Inventoryitems"));
	const FName NAME_InventoryChanged(TEXT("Oninventorychanged"));
	const FName NAME_SpawnedPlant(TEXT("Spawnedplant"));
	const FName NAME_IsReady(TEXT("Isready"));

	/** Where an inventory lives and how its slot struct is laid out. */
	struct FInventoryView
	{
		UObject* Interface = nullptr;           // set if the interface is implemented
		UObject* Owner = nullptr;               // component holding the array
		FArrayProperty* Array = nullptr;
		FIntProperty* CountProp = nullptr;
		FObjectPropertyBase* InstanceProp = nullptr;

		bool IsValid() const { return Interface || (Owner && Array && CountProp && InstanceProp); }
	};

	bool ImplementsInventory(const UObject* Object)
	{
		return Object && Object->GetClass()->ImplementsInterface(UMutateoInventoryInterface::StaticClass());
	}

	bool TryReflectInventory(UObject* Object, FInventoryView& Out)
	{
		if (!Object) return false;
		FArrayProperty* Array = FindFProperty<FArrayProperty>(Object->GetClass(), NAME_InventoryArray);
		if (!Array) return false;
		const FStructProperty* Inner = CastField<FStructProperty>(Array->Inner);
		if (!Inner || !Inner->Struct) return false;

		FIntProperty* Count = nullptr;
		FObjectPropertyBase* Instance = nullptr;
		for (TFieldIterator<FProperty> It(Inner->Struct); It; ++It)
		{
			const FString Name = It->GetName();
			if (!Count && Name.StartsWith(TEXT("count"), ESearchCase::IgnoreCase)) Count = CastField<FIntProperty>(*It);
			if (!Instance && Name.StartsWith(TEXT("iteminstance"), ESearchCase::IgnoreCase)) Instance = CastField<FObjectPropertyBase>(*It);
		}
		if (!Count || !Instance) return false;

		Out.Owner = Object;
		Out.Array = Array;
		Out.CountProp = Count;
		Out.InstanceProp = Instance;
		return true;
	}

	FInventoryView FindInventoryView(AActor* Owner)
	{
		FInventoryView View;
		if (!Owner) return View;

		TArray<AActor*, TInlineAllocator<3>> Candidates;
		Candidates.Add(Owner);
		if (const APawn* Pawn = Cast<APawn>(Owner))
		{
			if (Pawn->GetController()) Candidates.Add(Pawn->GetController());
		}
		else if (const AController* Controller = Cast<AController>(Owner))
		{
			if (Controller->GetPawn()) Candidates.Add(Controller->GetPawn());
		}

		// Interface first (explicit wins), then reflection.
		for (AActor* Candidate : Candidates)
		{
			if (ImplementsInventory(Candidate)) { View.Interface = Candidate; return View; }
			for (UActorComponent* Component : Candidate->GetComponents())
			{
				if (ImplementsInventory(Component)) { View.Interface = Component; return View; }
			}
		}
		for (AActor* Candidate : Candidates)
		{
			if (TryReflectInventory(Candidate, View)) return View;
			for (UActorComponent* Component : Candidate->GetComponents())
			{
				if (TryReflectInventory(Component, View)) return View;
			}
		}
		return View;
	}

	void BroadcastInventoryChanged(UObject* Owner)
	{
		const FMulticastDelegateProperty* Prop = FindFProperty<FMulticastDelegateProperty>(Owner->GetClass(), NAME_InventoryChanged);
		if (!Prop || !Prop->SignatureFunction || Prop->SignatureFunction->ParmsSize != 0) return;

		if (const FMulticastScriptDelegate* Delegate = Prop->GetMulticastDelegate(Prop->ContainerPtrToValuePtr<void>(Owner)))
		{
			Delegate->ProcessDelegate<UObject>(nullptr);
		}
	}

	AActor* GetSpawnedPlant(const AActor* Plot)
	{
		const FObjectPropertyBase* Prop = FindFProperty<FObjectPropertyBase>(Plot->GetClass(), NAME_SpawnedPlant);
		return Prop ? Cast<AActor>(Prop->GetObjectPropertyValue_InContainer(Plot)) : nullptr;
	}

	/** Plants are any actor with an "Isready" bool. */
	const FBoolProperty* GetReadyProp(const AActor* Actor)
	{
		return Actor ? FindFProperty<FBoolProperty>(Actor->GetClass(), NAME_IsReady) : nullptr;
	}

	/** Plants sitting on this plot: the one in Spawnedplant (if the plot sets it) plus any plant actor within a short radius. */
	void GetPlantsOnPlot(const AActor* Plot, TArray<AActor*>& OutPlants)
	{
		OutPlants.Reset();
		if (AActor* Tracked = GetSpawnedPlant(Plot))
		{
			if (IsValid(Tracked)) OutPlants.Add(Tracked);
		}

		const float Radius = 120.f;
		const FVector PlotLocation = Plot->GetActorLocation();
		for (TActorIterator<AActor> It(Plot->GetWorld()); It; ++It)
		{
			AActor* Actor = *It;
			if (!IsValid(Actor) || Actor == Plot || OutPlants.Contains(Actor) || !GetReadyProp(Actor)) continue;
			if (FVector::DistSquared2D(Actor->GetActorLocation(), PlotLocation) <= FMath::Square(Radius))
			{
				OutPlants.Add(Actor);
			}
		}
	}

	bool ImplementsPlot(const AActor* Actor)
	{
		return Actor && Actor->GetClass()->ImplementsInterface(UMutateoCropPlotInterface::StaticClass());
	}
}

// ------------------------------------------------------------------------------------------
// Inventory

bool MutateoBridge::HasInventory(AActor* Owner)
{
	return FindInventoryView(Owner).IsValid();
}

int32 MutateoBridge::CountItems(AActor* Owner, TSubclassOf<UItemDefinition> Item)
{
	if (!Item) return 0;
	const FInventoryView View = FindInventoryView(Owner);
	if (View.Interface) return IMutateoInventoryInterface::Execute_CountItemsOfType(View.Interface, Item);
	if (!View.IsValid()) return 0;

	int32 Total = 0;
	FScriptArrayHelper Helper(View.Array, View.Array->ContainerPtrToValuePtr<void>(View.Owner));
	for (int32 i = 0; i < Helper.Num(); ++i)
	{
		const uint8* Slot = Helper.GetRawPtr(i);
		const UItemInstance* Instance = Cast<UItemInstance>(View.InstanceProp->GetObjectPropertyValue_InContainer(Slot));
		if (Instance && Instance->ItemDefinition == Item)
		{
			Total += FMath::Max(0, View.CountProp->GetPropertyValue_InContainer(Slot));
		}
	}
	return Total;
}

bool MutateoBridge::ConsumeItems(AActor* Owner, TSubclassOf<UItemDefinition> Item, int32 Count)
{
	if (!Item || Count <= 0) return false;
	const FInventoryView View = FindInventoryView(Owner);
	if (View.Interface) return IMutateoInventoryInterface::Execute_ConsumeItemsOfType(View.Interface, Item, Count);
	if (!View.IsValid() || CountItems(Owner, Item) < Count) return false;

	int32 Remaining = Count;
	FScriptArrayHelper Helper(View.Array, View.Array->ContainerPtrToValuePtr<void>(View.Owner));
	// Take from the last stacks first so the first slots stay full.
	for (int32 i = Helper.Num() - 1; i >= 0 && Remaining > 0; --i)
	{
		uint8* Slot = Helper.GetRawPtr(i);
		const UItemInstance* Instance = Cast<UItemInstance>(View.InstanceProp->GetObjectPropertyValue_InContainer(Slot));
		if (!Instance || Instance->ItemDefinition != Item) continue;

		const int32 Have = View.CountProp->GetPropertyValue_InContainer(Slot);
		const int32 Take = FMath::Min(Have, Remaining);
		const int32 Left = Have - Take;
		Remaining -= Take;

		if (Left <= 0)
		{
			Helper.RemoveValues(i, 1); // same as the inventory's own drop logic (Array Remove)
		}
		else
		{
			View.CountProp->SetPropertyValue_InContainer(Slot, Left);
		}
	}

	BroadcastInventoryChanged(View.Owner);
	return Remaining == 0;
}

// ------------------------------------------------------------------------------------------
// Plots

bool MutateoBridge::IsPlot(const AActor* Actor)
{
	if (!Actor) return false;
	return ImplementsPlot(Actor) || FindFProperty<FObjectPropertyBase>(Actor->GetClass(), NAME_SpawnedPlant) != nullptr;
}

bool MutateoBridge::PlotHasCrop(AActor* Plot)
{
	if (!IsValid(Plot)) return false;
	if (ImplementsPlot(Plot)) return IMutateoCropPlotInterface::Execute_HasCrop(Plot);
	TArray<AActor*> Plants;
	GetPlantsOnPlot(Plot, Plants);
	return Plants.Num() > 0;
}

bool MutateoBridge::PlotIsCropReady(AActor* Plot)
{
	if (!IsValid(Plot)) return false;
	if (ImplementsPlot(Plot)) return IMutateoCropPlotInterface::Execute_IsCropReady(Plot);

	TArray<AActor*> Plants;
	GetPlantsOnPlot(Plot, Plants);
	for (const AActor* Plant : Plants)
	{
		const FBoolProperty* Ready = GetReadyProp(Plant);
		if (Ready && Ready->GetPropertyValue_InContainer(Plant)) return true;
	}
	return false;
}

void MutateoBridge::PlotClearCrop(AActor* Plot)
{
	if (!IsValid(Plot)) return;
	if (ImplementsPlot(Plot))
	{
		IMutateoCropPlotInterface::Execute_ClearCrop(Plot);
		return;
	}

	TArray<AActor*> Plants;
	GetPlantsOnPlot(Plot, Plants);
	for (AActor* Plant : Plants)
	{
		Plant->Destroy();
	}
	if (const FObjectPropertyBase* Prop = FindFProperty<FObjectPropertyBase>(Plot->GetClass(), NAME_SpawnedPlant))
	{
		Prop->SetObjectPropertyValue_InContainer(Plot, nullptr);
	}
}

void MutateoBridge::GetAllPlots(const UObject* WorldContext, TArray<AActor*>& OutPlots)
{
	OutPlots.Reset();
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World) return;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (IsPlot(*It)) OutPlots.Add(*It);
	}
}
