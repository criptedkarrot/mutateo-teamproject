#include "MutateoSubtitleWidget.h"

#include "MutateoGameSubsystem.h"
#include "MutateoLoopLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"

#define LOCTEXT_NAMESPACE "MutateoLoop"

namespace
{
	UTextBlock* AddText(UWidgetTree* Tree, UCanvasPanel* Canvas, FName Name, int32 FontSize, const FLinearColor& Color,
	                    const FAnchors& Anchors, const FVector2D& Alignment, const FVector2D& Position,
	                    const FVector2D& Size, ETextJustify::Type Justify)
	{
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
		FSlateFontInfo Font = Text->GetFont();
		Font.Size = FontSize;
		Text->SetFont(Font);
		Text->SetColorAndOpacity(FSlateColor(Color));
		Text->SetShadowOffset(FVector2D(2.f, 2.f));
		Text->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f));
		Text->SetJustification(Justify);
		Text->SetAutoWrapText(Size.X > 0.f);

		UCanvasPanelSlot* Slot = Canvas->AddChildToCanvas(Text);
		Slot->SetAnchors(Anchors);
		Slot->SetAlignment(Alignment);
		Slot->SetPosition(Position);
		if (Size.X > 0.f)
		{
			Slot->SetAutoSize(false);
			Slot->SetSize(Size);
		}
		else
		{
			Slot->SetAutoSize(true);
		}
		return Text;
	}
}

void UMutateoSubtitleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// No designer layout (plain C++ class) -> build a simple one.
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Canvas;

		SubtitleText = AddText(WidgetTree, Canvas, TEXT("SubtitleText"), 26, FLinearColor::White,
			FAnchors(0.5f, 1.f), FVector2D(0.5f, 1.f), FVector2D(0.f, -110.f), FVector2D(1200.f, 110.f), ETextJustify::Center);

		ObjectiveText = AddText(WidgetTree, Canvas, TEXT("ObjectiveText"), 20, FLinearColor(1.f, 0.85f, 0.35f),
			FAnchors(0.f, 0.f), FVector2D(0.f, 0.f), FVector2D(40.f, 40.f), FVector2D::ZeroVector, ETextJustify::Left);

		CoinsText = AddText(WidgetTree, Canvas, TEXT("CoinsText"), 22, FLinearColor(1.f, 0.9f, 0.2f),
			FAnchors(1.f, 0.f), FVector2D(1.f, 0.f), FVector2D(-40.f, 40.f), FVector2D::ZeroVector, ETextJustify::Right);
	}
}

void UMutateoSubtitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SubtitleText) SubtitleText->SetVisibility(ESlateVisibility::Collapsed);

	if (UMutateoGameSubsystem* Subsystem = UMutateoLoopLibrary::GetMutateoSubsystem(this))
	{
		Subsystem->OnCoinsChanged.AddUniqueDynamic(this, &UMutateoSubtitleWidget::HandleCoinsChanged);
		SetCoins(Subsystem->GetCoins());
	}
}

void UMutateoSubtitleWidget::NativeDestruct()
{
	if (UMutateoGameSubsystem* Subsystem = UMutateoLoopLibrary::GetMutateoSubsystem(this))
	{
		Subsystem->OnCoinsChanged.RemoveDynamic(this, &UMutateoSubtitleWidget::HandleCoinsChanged);
	}
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HideTimer);
	}
	Super::NativeDestruct();
}

void UMutateoSubtitleWidget::ShowSubtitle(const FText& Text, float Duration)
{
	if (!SubtitleText) return;

	SubtitleText->SetText(Text);
	SubtitleText->SetVisibility(ESlateVisibility::HitTestInvisible);
	OnSubtitleShown(Text);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(HideTimer, this, &UMutateoSubtitleWidget::HideSubtitle, FMath::Max(0.5f, Duration), false);
	}
}

void UMutateoSubtitleWidget::HideSubtitle()
{
	if (SubtitleText) SubtitleText->SetVisibility(ESlateVisibility::Collapsed);
	OnSubtitleHidden();
}

void UMutateoSubtitleWidget::SetObjective(const FText& Text)
{
	if (ObjectiveText) ObjectiveText->SetText(Text);
}

void UMutateoSubtitleWidget::SetCoins(int32 Coins)
{
	if (CoinsText) CoinsText->SetText(FText::Format(LOCTEXT("Coins", "Coins: {0}"), FText::AsNumber(Coins)));
}

void UMutateoSubtitleWidget::HandleCoinsChanged(int32 NewCoins)
{
	SetCoins(NewCoins);
}

#undef LOCTEXT_NAMESPACE
