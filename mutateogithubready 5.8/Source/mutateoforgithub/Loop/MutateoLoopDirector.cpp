#include "MutateoLoopDirector.h"

#include "MutateoCraftingStation.h"
#include "MutateoCropPlotInterface.h"
#include "MutateoDepositStation.h"
#include "MutateoHarvestSwitch.h"
#include "MutateoUpgradeStation.h"
#include "MutateoGameSubsystem.h"
#include "MutateoLoopLibrary.h"
#include "MutateoMutant.h"
#include "MutateoProjectBridge.h"
#include "MutateoSubtitleWidget.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "InputCoreTypes.h"
#include "NiagaraSystem.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "MutateoLoop"

AMutateoLoopDirector::AMutateoLoopDirector()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	FireKey = EKeys::LeftMouseButton;
	SetupDefaultLines();
}

void AMutateoLoopDirector::SetupDefaultLines()
{
	using S = EMutateoLoopStage;

	FirstLoopLines.Add(S::Intro,            FMutateoVoiceLine(LOCTEXT("L_Intro", "Oh hey, welcome farmer! Let's get you started straight away.")));
	FirstLoopLines.Add(S::CollectSeeds,     FMutateoVoiceLine(LOCTEXT("L_Collect", "See that glowing bundle right in front of you? Grab it.")));
	FirstLoopLines.Add(S::PlantSeeds,       FMutateoVoiceLine(LOCTEXT("L_Plant", "Perfect. Now go and plant them in the plots over at the farm.")));
	FirstLoopLines.Add(S::FetchMaterials,   FMutateoVoiceLine(LOCTEXT("L_Fetch", "While those grow, I need you to find a few bits for me. Scrap, rope and plastic. They're scattered around the island.")));
	FirstLoopLines.Add(S::CraftWeapon,      FMutateoVoiceLine(LOCTEXT("L_Craft", "That's everything! Bring it over to the crafting station.")));
	FirstLoopLines.Add(S::WaitForCrops,     FMutateoVoiceLine(LOCTEXT("L_Wait", "There you go, your very own weapon. Trust me, you're going to need it.")));
	FirstLoopLines.Add(S::ActivateSwitch,   FMutateoVoiceLine(LOCTEXT("L_Switch", "Oh, your crops are ready to harvest! Hit the switch next to the farm.")));
	FirstLoopLines.Add(S::DefeatMutants,    FMutateoVoiceLine(LOCTEXT("L_Fight", "Uh oh. Those aren't quite carrots any more... Take them out!")));
	FirstLoopLines.Add(S::DepositResources, FMutateoVoiceLine(LOCTEXT("L_Deposit", "Nice work! Grab what they dropped and hand it in at the deposit station.")));
	FirstLoopLines.Add(S::VisitUpgrades,    FMutateoVoiceLine(LOCTEXT("L_Upgrade", "Coins! Head over to the upgrade station and treat yourself.")));
	FirstLoopLines.Add(S::Replant,          FMutateoVoiceLine(LOCTEXT("L_Replant", "Now plant whatever you've got left and we'll go again.")));

	RepeatLoopLines.Add(S::ActivateSwitch,   FMutateoVoiceLine(LOCTEXT("R_Switch", "Crops are ready. You know what to do.")));
	RepeatLoopLines.Add(S::DefeatMutants,    FMutateoVoiceLine(LOCTEXT("R_Fight", "Here they come!")));
	RepeatLoopLines.Add(S::DepositResources, FMutateoVoiceLine(LOCTEXT("R_Deposit", "Get those drops to the deposit station.")));
	RepeatLoopLines.Add(S::VisitUpgrades,    FMutateoVoiceLine(LOCTEXT("R_Upgrade", "More coins. Fancy an upgrade?")));
	RepeatLoopLines.Add(S::Replant,          FMutateoVoiceLine(LOCTEXT("R_Replant", "Replant and keep it going.")));

	MissingMaterialsLine = FMutateoVoiceLine(LOCTEXT("L_Missing", "You're still missing some materials. Keep looking!"));
	NeedWeaponLine       = FMutateoVoiceLine(LOCTEXT("L_NeedWeapon", "Hold on, you'll want a weapon before you do that."));
	NothingReadyLine     = FMutateoVoiceLine(LOCTEXT("L_NotReady", "Nothing's ready to harvest yet. Give it a moment."));
	NothingToDepositLine = FMutateoVoiceLine(LOCTEXT("L_NoDeposit", "You've got nothing to hand in. Pick up what the mutants dropped."));
}

// ---------------------------------------------------------------------------------------------

void AMutateoLoopDirector::BeginPlay()
{
	Super::BeginPlay();

	if (UMutateoGameSubsystem* Subsystem = GetWorld()->GetSubsystem<UMutateoGameSubsystem>())
	{
		Subsystem->SetDirector(this);
	}

	if (StartDelay > 0.f)
	{
		GetWorldTimerManager().SetTimer(StartTimer, this, &AMutateoLoopDirector::StartLoop, StartDelay, false);
	}
	else
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &AMutateoLoopDirector::StartLoop);
	}
}

void AMutateoLoopDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearHighlights();
	GetWorldTimerManager().ClearAllTimersForObject(this);
	if (SubtitleWidget)
	{
		SubtitleWidget->RemoveFromParent();
		SubtitleWidget = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AMutateoLoopDirector::StartLoop()
{
	BindCombatInput();

	{
		const TSubclassOf<UMutateoSubtitleWidget> HudClass = SubtitleWidgetClass ? SubtitleWidgetClass : TSubclassOf<UMutateoSubtitleWidget>(UMutateoSubtitleWidget::StaticClass());
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			SubtitleWidget = CreateWidget<UMutateoSubtitleWidget>(PC, HudClass);
			if (SubtitleWidget)
			{
				SubtitleWidget->AddToViewport(10);
			}
		}
	}

	GetWorldTimerManager().SetTimer(PollTimer, this, &AMutateoLoopDirector::EvaluateStage, 0.25f, true);

	if (bPlayTutorial)
	{
		EnterStage(EMutateoLoopStage::Intro);
	}
	else
	{
		bWeaponCrafted = true;
		LoopsCompleted = 1; // use the short repeat lines
		EnterStage(EMutateoLoopStage::Replant);
	}
}

// ---------------------------------------------------------------------------------------------
// Stage machine

void AMutateoLoopDirector::EnterStage(EMutateoLoopStage NewStage)
{
	const EMutateoLoopStage OldStage = Stage;

	// A loop is complete when the player replants after visiting the upgrade station.
	if (OldStage == EMutateoLoopStage::Replant && NewStage == EMutateoLoopStage::WaitForCrops)
	{
		++LoopsCompleted;
		OnLoopCompleted.Broadcast(LoopsCompleted);
	}

	Stage = NewStage;
	PlayStageLine(NewStage);
	RefreshHighlights();
	RefreshObjective();
	OnStageChanged.Broadcast(Stage);

	if (Stage == EMutateoLoopStage::Intro)
	{
		const float IntroLength = FMath::Max(0.5f, GetWorldTimerManager().GetTimerRemaining(VoiceTimer));
		FTimerDelegate Next = FTimerDelegate::CreateUObject(this, &AMutateoLoopDirector::EnterStage, EMutateoLoopStage::CollectSeeds);
		GetWorldTimerManager().SetTimer(IntroTimer, Next, IntroLength, false);
	}
}

void AMutateoLoopDirector::EvaluateStage()
{
	using S = EMutateoLoopStage;

	switch (Stage)
	{
	case S::CollectSeeds:
		if (HasCollectedStarter()) EnterStage(S::PlantSeeds);
		break;

	case S::PlantSeeds:
		if (AnyCropPlanted()) EnterStage(S::FetchMaterials);
		break;

	case S::FetchMaterials:
		if (bWeaponCrafted) EnterStage(S::WaitForCrops);
		else if (AreFetchRequirementsMet()) EnterStage(S::CraftWeapon);
		break;

	case S::CraftWeapon:
		if (bWeaponCrafted) EnterStage(S::WaitForCrops);
		else if (!AreFetchRequirementsMet()) EnterStage(S::FetchMaterials);
		break;

	case S::WaitForCrops:
		if (AnyCropReady()) EnterStage(S::ActivateSwitch);
		break;

	case S::DefeatMutants:
		if (GetAliveMutantCount() == 0) EnterStage(S::DepositResources);
		break;

	case S::Replant:
		if (AnyCropPlanted()) EnterStage(S::WaitForCrops);
		break;

	default:
		break;
	}

	RefreshObjective();
}

void AMutateoLoopDirector::DebugSkipToStage(EMutateoLoopStage NewStage, bool bMarkWeaponCrafted)
{
	GetWorldTimerManager().ClearTimer(IntroTimer);
	if (bMarkWeaponCrafted) bWeaponCrafted = true;
	EnterStage(NewStage);
}

void AMutateoLoopDirector::NotifyWeaponCrafted()
{
	bWeaponCrafted = true;
	EvaluateStage();
}

void AMutateoLoopDirector::NotifyMutantsReleased(const TArray<AMutateoMutant*>& Mutants)
{
	for (AMutateoMutant* Mutant : Mutants)
	{
		if (Mutant) ActiveMutants.Add(Mutant);
	}
	if (Mutants.Num() > 0)
	{
		EnterStage(EMutateoLoopStage::DefeatMutants);
	}
}

void AMutateoLoopDirector::NotifyDeposited(int32 CoinsEarned)
{
	if (CoinsEarned > 0 && Stage == EMutateoLoopStage::DepositResources)
	{
		EnterStage(EMutateoLoopStage::VisitUpgrades);
	}
}

void AMutateoLoopDirector::NotifyUpgradeStationVisited()
{
	if (Stage == EMutateoLoopStage::VisitUpgrades)
	{
		EnterStage(EMutateoLoopStage::Replant);
	}
}

bool AMutateoLoopDirector::CanActivateHarvestSwitch(FMutateoVoiceLine& OutReason) const
{
	if (!bWeaponCrafted)
	{
		OutReason = NeedWeaponLine;
		return false;
	}
	if (Stage == EMutateoLoopStage::DefeatMutants || !AnyCropReady())
	{
		OutReason = NothingReadyLine;
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------------------------
// World queries

TArray<AActor*> AMutateoLoopDirector::GetPlots() const
{
	TArray<AActor*> Result;
	for (AActor* Plot : PlantPlots)
	{
		if (IsValid(Plot)) Result.Add(Plot);
	}
	if (Result.IsEmpty())
	{
		MutateoBridge::GetAllPlots(this, Result);
	}
	return Result;
}

bool AMutateoLoopDirector::AnyCropPlanted() const
{
	for (AActor* Plot : GetPlots())
	{
		if (MutateoBridge::PlotHasCrop(Plot))
		{
			return true;
		}
	}
	return false;
}

bool AMutateoLoopDirector::AnyCropReady() const
{
	for (AActor* Plot : GetPlots())
	{
		if (MutateoBridge::PlotIsCropReady(Plot))
		{
			return true;
		}
	}
	return false;
}

int32 AMutateoLoopDirector::GetAliveMutantCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AMutateoMutant>& Mutant : ActiveMutants)
	{
		if (Mutant.IsValid() && !Mutant->IsDead()) ++Count;
	}
	return Count;
}

bool AMutateoLoopDirector::HasCollectedStarter() const
{
	if (StarterItem && UMutateoLoopLibrary::GetInventoryItemCount(GetPlayerPawn(), StarterItem) > 0)
	{
		return true;
	}

	// Fallback: every starter pickup has been picked up (destroyed).
	if (StarterPickups.IsEmpty()) return false;
	for (AActor* Pickup : StarterPickups)
	{
		if (IsValid(Pickup)) return false;
	}
	return true;
}

bool AMutateoLoopDirector::AreFetchRequirementsMet() const
{
	return !CraftingStation || CraftingStation->HasAllMaterials(GetPlayerPawn());
}

APawn* AMutateoLoopDirector::GetPlayerPawn() const
{
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

// ---------------------------------------------------------------------------------------------
// Highlights and objective

void AMutateoLoopDirector::ClearHighlights()
{
	for (const TWeakObjectPtr<AActor>& Actor : HighlightedActors)
	{
		UMutateoLoopLibrary::SetActorHighlighted(Actor.Get(), false, nullptr);
	}
	HighlightedActors.Reset();
}

void AMutateoLoopDirector::RefreshHighlights()
{
	ClearHighlights();

	TArray<AActor*> Targets;
	switch (Stage)
	{
	case EMutateoLoopStage::CollectSeeds:     for (AActor* A : StarterPickups) Targets.Add(A); break;
	case EMutateoLoopStage::PlantSeeds:
	case EMutateoLoopStage::Replant:          Targets = GetPlots(); break;
	case EMutateoLoopStage::FetchMaterials:   for (AActor* A : FetchPickups) Targets.Add(A); break;
	case EMutateoLoopStage::CraftWeapon:      Targets.Add(CraftingStation.Get()); break;
	case EMutateoLoopStage::ActivateSwitch:   Targets.Add(HarvestSwitch.Get()); break;
	case EMutateoLoopStage::DepositResources: Targets.Add(DepositStation.Get()); break;
	case EMutateoLoopStage::VisitUpgrades:    Targets.Add(UpgradeStation.Get()); break;
	default: break;
	}

	for (AActor* Target : Targets)
	{
		if (!IsValid(Target)) continue;
		UMutateoLoopLibrary::SetActorHighlighted(Target, true, HighlightMaterial);
		HighlightedActors.Add(Target);
	}
}

void AMutateoLoopDirector::RefreshObjective()
{
	const FText NewObjective = BuildObjectiveText();
	if (NewObjective.EqualTo(CurrentObjective)) return;

	CurrentObjective = NewObjective;
	if (SubtitleWidget) SubtitleWidget->SetObjective(CurrentObjective);
	OnObjectiveChanged.Broadcast(CurrentObjective);
}

FText AMutateoLoopDirector::BuildObjectiveText() const
{
	switch (Stage)
	{
	case EMutateoLoopStage::CollectSeeds:     return LOCTEXT("O_Collect", "Pick up the glowing crop");
	case EMutateoLoopStage::PlantSeeds:       return LOCTEXT("O_Plant", "Plant your crops at the farm plots");
	case EMutateoLoopStage::CraftWeapon:      return LOCTEXT("O_Craft", "Hand in the materials at the crafting station");
	case EMutateoLoopStage::WaitForCrops:     return LOCTEXT("O_Wait", "Wait for your crops to grow");
	case EMutateoLoopStage::ActivateSwitch:   return LOCTEXT("O_Switch", "Activate the harvest switch");
	case EMutateoLoopStage::DefeatMutants:
		return FText::Format(LOCTEXT("O_Fight", "Defeat the mutants ({0} left)"), FText::AsNumber(GetAliveMutantCount()));
	case EMutateoLoopStage::DepositResources: return LOCTEXT("O_Deposit", "Collect the drops and hand them in at the deposit station");
	case EMutateoLoopStage::VisitUpgrades:    return LOCTEXT("O_Upgrade", "Visit the upgrade station");
	case EMutateoLoopStage::Replant:          return LOCTEXT("O_Replant", "Replant your remaining crops");

	case EMutateoLoopStage::FetchMaterials:
	{
		if (!CraftingStation) return LOCTEXT("O_FetchGeneric", "Find crafting materials");

		APawn* Player = GetPlayerPawn();
		TArray<FString> Parts;
		for (const FMutateoItemRequirement& Req : CraftingStation->Recipe)
		{
			if (!Req.Item) continue;
			const int32 Have = FMath::Min(UMutateoLoopLibrary::GetInventoryItemCount(Player, Req.Item), Req.Count);
			FString Name = Req.Item.GetDefaultObject()->ItemName.ToString();
			if (Name.IsEmpty())
			{
				Name = Req.Item->GetName();
				Name.RemoveFromEnd(TEXT("_C"));
			}
			Parts.Add(FString::Printf(TEXT("%s %d/%d"), *Name, Have, Req.Count));
		}
		return FText::Format(LOCTEXT("O_Fetch", "Find materials: {0}"), FText::FromString(FString::Join(Parts, TEXT(", "))));
	}

	default:
		return FText::GetEmpty();
	}
}

// ---------------------------------------------------------------------------------------------
// Voice

void AMutateoLoopDirector::PlayStageLine(EMutateoLoopStage ForStage)
{
	const TMap<EMutateoLoopStage, FMutateoVoiceLine>& Lines = LoopsCompleted == 0 ? FirstLoopLines : RepeatLoopLines;
	if (const FMutateoVoiceLine* Line = Lines.Find(ForStage))
	{
		PlayVoiceLine(*Line);
	}
}

float AMutateoLoopDirector::GetLineDuration(const FMutateoVoiceLine& Line) const
{
	if (Line.Duration > 0.f) return Line.Duration;
	if (Line.Sound) return FMath::Max(1.f, Line.Sound->GetDuration());

	TArray<FString> Words;
	Line.Text.ToString().ParseIntoArrayWS(Words);
	return FMath::Max(2.5f, 1.f + Words.Num() * 0.38f);
}

float AMutateoLoopDirector::PlayVoiceLine(const FMutateoVoiceLine& Line)
{
	if (Line.IsEmpty()) return 0.f;

	float Wait = FMath::Max(0.f, GetWorldTimerManager().GetTimerRemaining(VoiceTimer));
	for (const FMutateoVoiceLine& Queued : VoiceQueue)
	{
		Wait += GetLineDuration(Queued) + VoiceLineGap;
	}

	VoiceQueue.Add(Line);
	if (!GetWorldTimerManager().IsTimerActive(VoiceTimer))
	{
		PlayNextQueuedLine();
	}
	return Wait + GetLineDuration(Line);
}

void AMutateoLoopDirector::PlayNextQueuedLine()
{
	if (VoiceQueue.IsEmpty()) return;

	const FMutateoVoiceLine Line = VoiceQueue[0];
	VoiceQueue.RemoveAt(0);

	const float Duration = GetLineDuration(Line);
	if (Line.Sound)
	{
		UGameplayStatics::PlaySound2D(this, Line.Sound);
	}
	if (SubtitleWidget)
	{
		SubtitleWidget->ShowSubtitle(Line.Text, Duration);
	}
	OnVoiceLine.Broadcast(Line.Text, Duration);

	GetWorldTimerManager().SetTimer(VoiceTimer, this, &AMutateoLoopDirector::PlayNextQueuedLine, Duration + VoiceLineGap, false);
}

// ---------------------------------------------------------------------------------------------
// Combat

void AMutateoLoopDirector::BindCombatInput()
{
	if (!AttackAction) return;
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;

	EnableInput(PC);
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		Input->BindAction(AttackAction, ETriggerEvent::Started, this, &AMutateoLoopDirector::HandleAttackInput);
	}
}

AActor* AMutateoLoopDirector::GetEquippedWeapon() const
{
	const APawn* Player = GetPlayerPawn();
	if (!Player) return nullptr;
	TArray<AActor*> Attached;
	Player->GetAttachedActors(Attached);
	return Attached.Num() > 0 ? Attached[0] : nullptr;
}

void AMutateoLoopDirector::HandleAttackInput()
{
	// Started on press. Keep firing on a timer while FireKey is held.
	if (GetWorldTimerManager().IsTimerActive(FireTimer)) return;

	AActor* Weapon = GetEquippedWeapon();
	if (bRequireEquippedWeapon && (!bWeaponCrafted || !Weapon)) return;

	// Firing again before the stream despawned: reuse it.
	GetWorldTimerManager().ClearTimer(StreamDespawnTimer);
	if (StreamComponent && (!Weapon || StreamComponent->GetAttachParent() != Weapon->GetRootComponent()))
	{
		DespawnStream(); // gun changed; start fresh
	}
	if (StreamComponent)
	{
		StreamComponent->Activate(true);
	}
	else if (MuzzleEffect && Weapon)
	{
		StreamComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			MuzzleEffect, Weapon->GetRootComponent(), NAME_None, StreamLocationOffset, StreamRotationOffset,
			EAttachLocation::KeepRelativeOffset, /*bAutoDestroy*/ false);
	}

	FireOnce();
	GetWorldTimerManager().SetTimer(FireTimer, this, &AMutateoLoopDirector::FireTick, FireInterval, true);
}

void AMutateoLoopDirector::FireTick()
{
	const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	const bool bHeld = PC && PC->IsInputKeyDown(FireKey.IsValid() ? FireKey : EKeys::LeftMouseButton);
	if (!bHeld || (bRequireEquippedWeapon && !GetEquippedWeapon()))
	{
		StopFiring();
		return;
	}
	FireOnce();
}

void AMutateoLoopDirector::FireOnce()
{
	APawn* Player = GetPlayerPawn();
	if (!Player) return;

	FHitResult Hit;
	UMutateoLoopLibrary::FireWeapon(Player, Hit, WeaponDamage, WeaponRange, AimAssistRadius);

	// Point the stream at what we're hitting.
	if (StreamComponent)
	{
		const FVector From = StreamComponent->GetComponentLocation();
		StreamComponent->SetWorldRotation((Hit.ImpactPoint - From).Rotation() + StreamRotationOffset);
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (ImpactEffect && Hit.bBlockingHit && Now - LastImpactFxTime > 0.3f)
	{
		LastImpactFxTime = Now;
		if (UNiagaraComponent* Splash = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation()))
		{
			SpawnedImpacts.Emplace(Splash, Now);
			if (!GetWorldTimerManager().IsTimerActive(ImpactCleanupTimer))
			{
				GetWorldTimerManager().SetTimer(ImpactCleanupTimer, this, &AMutateoLoopDirector::CleanupImpactEffects, 0.25f, true);
			}
		}
	}
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, Player->GetActorLocation());
	}
}

void AMutateoLoopDirector::StopFiring()
{
	GetWorldTimerManager().ClearTimer(FireTimer);
	if (StreamComponent)
	{
		StreamComponent->Deactivate();
		if (EffectDespawnTime > 0.f)
		{
			GetWorldTimerManager().SetTimer(StreamDespawnTimer, this, &AMutateoLoopDirector::DespawnStream, EffectDespawnTime, false);
		}
		else
		{
			DespawnStream();
		}
	}
}

void AMutateoLoopDirector::DespawnStream()
{
	GetWorldTimerManager().ClearTimer(StreamDespawnTimer);
	if (StreamComponent)
	{
		StreamComponent->DestroyComponent();
		StreamComponent = nullptr;
	}
}

void AMutateoLoopDirector::CleanupImpactEffects()
{
	const float Now = GetWorld()->GetTimeSeconds();
	for (int32 i = SpawnedImpacts.Num() - 1; i >= 0; --i)
	{
		UNiagaraComponent* Splash = SpawnedImpacts[i].Key.Get();
		if (!Splash)
		{
			SpawnedImpacts.RemoveAtSwap(i);
		}
		else if (Now - SpawnedImpacts[i].Value >= EffectDespawnTime)
		{
			Splash->DestroyComponent();
			SpawnedImpacts.RemoveAtSwap(i);
		}
	}
	if (SpawnedImpacts.IsEmpty())
	{
		GetWorldTimerManager().ClearTimer(ImpactCleanupTimer);
	}
}

#undef LOCTEXT_NAMESPACE
