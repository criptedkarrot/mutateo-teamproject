#include "MutateoUpgradeMenu.h"

#include "MutateoGameSubsystem.h"
#include "MutateoLoopLibrary.h"
#include "MutateoUpgradeStation.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"

#define LOCTEXT_NAMESPACE "MutateoLoop"

namespace
{
	UTextBlock* MakeMenuText(UWidgetTree* Tree, const FText& Text, int32 Size, const FLinearColor& Color = FLinearColor::White)
	{
		UTextBlock* Block = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		FSlateFontInfo Font = Block->GetFont();
		Font.Size = Size;
		Block->SetFont(Font);
		Block->SetText(Text);
		Block->SetColorAndOpacity(FSlateColor(Color));
		return Block;
	}

	UButton* MakeButton(UWidgetTree* Tree, UTextBlock*& OutLabel, const FText& Label)
	{
		UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass());
		OutLabel = MakeMenuText(Tree, Label, 16, FLinearColor::Black);
		Button->AddChild(OutLabel);
		return Button;
	}
}

// ------------------------------------------------------------------------------------------
// Row

void UMutateoUpgradeRow::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!WidgetTree || WidgetTree->RootWidget) return;

	UHorizontalBox* Box = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Box;

	UVerticalBox* TextColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	NameText = MakeMenuText(WidgetTree, FText::GetEmpty(), 20, FLinearColor(1.f, 0.9f, 0.5f));
	DescriptionText = MakeMenuText(WidgetTree, FText::GetEmpty(), 14, FLinearColor(0.85f, 0.85f, 0.85f));
	TextColumn->AddChildToVerticalBox(NameText);
	TextColumn->AddChildToVerticalBox(DescriptionText);

	UHorizontalBoxSlot* TextSlot = Box->AddChildToHorizontalBox(TextColumn);
	TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	TextSlot->SetVerticalAlignment(VAlign_Center);

	UTextBlock* BuyLabel = nullptr;
	BuyButton = MakeButton(WidgetTree, BuyLabel, LOCTEXT("Buy", "Buy"));
	BuyText = BuyLabel;
	BuyButton->OnClicked.AddDynamic(this, &UMutateoUpgradeRow::HandleBuyClicked);
	UHorizontalBoxSlot* ButtonSlot = Box->AddChildToHorizontalBox(BuyButton);
	ButtonSlot->SetVerticalAlignment(VAlign_Center);
	ButtonSlot->SetPadding(FMargin(16.f, 0.f, 0.f, 0.f));
}

void UMutateoUpgradeRow::Setup(AMutateoUpgradeStation* InStation, AActor* InBuyer, FName InUpgradeId)
{
	Station = InStation;
	Buyer = InBuyer;
	UpgradeId = InUpgradeId;
	Refresh();
}

void UMutateoUpgradeRow::Refresh()
{
	const UMutateoGameSubsystem* Subsystem = UMutateoLoopLibrary::GetMutateoSubsystem(this);
	FMutateoUpgradeDef Def;
	if (!Subsystem || !Subsystem->GetUpgradeDefinition(UpgradeId, Def) || !NameText) return;

	const int32 Level = Subsystem->GetUpgradeLevel(UpgradeId);
	NameText->SetText(FText::Format(LOCTEXT("RowName", "{0}   Lv {1}/{2}"), Def.DisplayName, FText::AsNumber(Level), FText::AsNumber(Def.MaxLevel)));
	DescriptionText->SetText(Def.Description);

	const int32 Cost = Subsystem->GetUpgradeCost(UpgradeId);
	if (Cost < 0)
	{
		BuyText->SetText(LOCTEXT("Maxed", "MAX"));
		BuyButton->SetIsEnabled(false);
	}
	else
	{
		BuyText->SetText(FText::Format(LOCTEXT("BuyCost", "Buy ({0} coins)"), FText::AsNumber(Cost)));
		BuyButton->SetIsEnabled(Subsystem->GetCoins() >= Cost);
	}
}

void UMutateoUpgradeRow::HandleBuyClicked()
{
	if (AMutateoUpgradeStation* S = Station.Get())
	{
		S->PurchaseUpgrade(UpgradeId, Buyer.Get());
	}
	// Coins changed -> the menu refreshes every row.
}

// ------------------------------------------------------------------------------------------
// Menu

void UMutateoUpgradeMenu::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (!WidgetTree || WidgetTree->RootWidget) return;

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Canvas;

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Panel->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.92f));
	Panel->SetPadding(FMargin(28.f));
	UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Panel);
	PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	PanelSlot->SetAutoSize(true);

	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Panel->SetContent(Column);

	Column->AddChildToVerticalBox(MakeMenuText(WidgetTree, LOCTEXT("ShopTitle", "UPGRADE STATION"), 30));
	CoinsText = MakeMenuText(WidgetTree, FText::GetEmpty(), 18, FLinearColor(1.f, 0.85f, 0.2f));
	UVerticalBoxSlot* CoinsSlot = Column->AddChildToVerticalBox(CoinsText);
	CoinsSlot->SetPadding(FMargin(0.f, 4.f, 0.f, 16.f));

	RowBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	Column->AddChildToVerticalBox(RowBox);

	UTextBlock* CloseLabel = nullptr;
	CloseButton = MakeButton(WidgetTree, CloseLabel, LOCTEXT("Close", "Close"));
	CloseButton->OnClicked.AddDynamic(this, &UMutateoUpgradeMenu::HandleCloseClicked);
	UVerticalBoxSlot* CloseSlot = Column->AddChildToVerticalBox(CloseButton);
	CloseSlot->SetHorizontalAlignment(HAlign_Right);
	CloseSlot->SetPadding(FMargin(0.f, 16.f, 0.f, 0.f));
}

void UMutateoUpgradeMenu::Open(AMutateoUpgradeStation* InStation, AActor* InBuyer)
{
	if (!InStation || !RowBox) return;

	RowBox->ClearChildren();
	Rows.Reset();
	for (const FMutateoUpgradeDef& Def : InStation->GetUpgrades())
	{
		UMutateoUpgradeRow* Row = CreateWidget<UMutateoUpgradeRow>(GetOwningPlayer(), UMutateoUpgradeRow::StaticClass());
		if (!Row) continue;
		UVerticalBoxSlot* RowSlot = RowBox->AddChildToVerticalBox(Row);
		RowSlot->SetPadding(FMargin(0.f, 6.f));
		Row->Setup(InStation, InBuyer, Def.Id);
		Rows.Add(Row);
	}

	if (UMutateoGameSubsystem* Subsystem = UMutateoLoopLibrary::GetMutateoSubsystem(this))
	{
		Subsystem->OnCoinsChanged.AddUniqueDynamic(this, &UMutateoUpgradeMenu::HandleCoinsChanged);
	}
	RefreshAll();

	if (!IsInViewport()) AddToViewport(50);

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->SetShowMouseCursor(true);
		PC->SetIgnoreLookInput(true);
		PC->SetIgnoreMoveInput(true);
	}
}

void UMutateoUpgradeMenu::RefreshAll()
{
	if (const UMutateoGameSubsystem* Subsystem = UMutateoLoopLibrary::GetMutateoSubsystem(this))
	{
		if (CoinsText) CoinsText->SetText(FText::Format(LOCTEXT("MenuCoins", "Coins: {0}"), FText::AsNumber(Subsystem->GetCoins())));
	}
	for (UMutateoUpgradeRow* Row : Rows)
	{
		if (Row) Row->Refresh();
	}
}

void UMutateoUpgradeMenu::HandleCoinsChanged(int32 NewCoins)
{
	RefreshAll();
}

void UMutateoUpgradeMenu::HandleCloseClicked()
{
	CloseMenu();
}

void UMutateoUpgradeMenu::CloseMenu()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
		PC->ResetIgnoreLookInput();
		PC->ResetIgnoreMoveInput();
	}
	RemoveFromParent();
}

void UMutateoUpgradeMenu::NativeDestruct()
{
	if (UMutateoGameSubsystem* Subsystem = UMutateoLoopLibrary::GetMutateoSubsystem(this))
	{
		Subsystem->OnCoinsChanged.RemoveDynamic(this, &UMutateoUpgradeMenu::HandleCoinsChanged);
	}
	Super::NativeDestruct();
}

#undef LOCTEXT_NAMESPACE
