// FlagQuizWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "FlagQuizWidget.generated.h"

// ── Struct pour une paire drapeau/réponse ──────────────────────────
USTRUCT(BlueprintType)
struct FFlagQuizPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* FlagTexture = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CountryName;
};

// ── Struct pour une ligne de connexion dessinée ────────────────────
USTRUCT()
struct FQuizConnectionLine
{
	GENERATED_BODY()

	int32 FlagIndex = -1;
	int32 AnswerIndex = -1;
	FLinearColor LineColor = FLinearColor::White;
};

// ═══════════════════════════════════════════════════════════════════
//  UFlagQuizWidget — Widget principal du quiz drapeaux
// ═══════════════════════════════════════════════════════════════════
UCLASS()
class MARCISLAND_API UFlagQuizWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ── Configuration (à remplir dans l'éditeur ou en code) ────────
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quiz")
	TArray<FFlagQuizPair> QuizPairs;

	// ── Widgets bindés depuis le UMG Designer ──────────────────────
	// Drapeaux (boutons avec image)
	UPROPERTY(meta = (BindWidget))
	UButton* FlagButton_0;
	UPROPERTY(meta = (BindWidget))
	UButton* FlagButton_1;
	UPROPERTY(meta = (BindWidget))
	UButton* FlagButton_2;
	UPROPERTY(meta = (BindWidget))
	UButton* FlagButton_3;

	// Images des drapeaux (enfants des boutons)
	UPROPERTY(meta = (BindWidget))
	UImage* FlagImage_0;
	UPROPERTY(meta = (BindWidget))
	UImage* FlagImage_1;
	UPROPERTY(meta = (BindWidget))
	UImage* FlagImage_2;
	UPROPERTY(meta = (BindWidget))
	UImage* FlagImage_3;

	// Boutons réponses (avec texte)
	UPROPERTY(meta = (BindWidget))
	UButton* AnswerButton_0;
	UPROPERTY(meta = (BindWidget))
	UButton* AnswerButton_1;
	UPROPERTY(meta = (BindWidget))
	UButton* AnswerButton_2;
	UPROPERTY(meta = (BindWidget))
	UButton* AnswerButton_3;

	// Textes des réponses
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnswerText_0;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnswerText_1;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnswerText_2;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnswerText_3;

	// Bouton Submit
	UPROPERTY(meta = (BindWidget))
	UButton* SubmitButton;

	// Texte de feedback (ex: "3/4 bonnes réponses!")
	UPROPERTY(meta = (BindWidget))
	UTextBlock* FeedbackText;

protected:
	virtual void NativeConstruct() override;
	virtual int32 NativePaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled
	) const override;

private:
	// ── État interne ───────────────────────────────────────────────
	int32 SelectedFlagIndex = -1;
	bool bQuizSubmitted = false;

	// Map : FlagIndex → AnswerIndex (les liens du joueur)
	TMap<int32, int32> PlayerLinks;

	// Lignes à dessiner
	TArray<FQuizConnectionLine> ConnectionLines;

	// Arrays pour accès par index
	TArray<UButton*> FlagButtons;
	TArray<UButton*> AnswerButtons;
	TArray<UImage*> FlagImages;
	TArray<UTextBlock*> AnswerTexts;

	// Map des bonnes réponses : FlagIndex → nom correct du pays
	TMap<int32, FString> CorrectAnswers;

	// ── Fonctions internes ─────────────────────────────────────────
	void InitializeQuiz();
	void SetupBindings();
	void ShuffleAnswers();

	// Callbacks des clics
	UFUNCTION() void OnFlagClicked_0();
	UFUNCTION() void OnFlagClicked_1();
	UFUNCTION() void OnFlagClicked_2();
	UFUNCTION() void OnFlagClicked_3();

	UFUNCTION() void OnAnswerClicked_0();
	UFUNCTION() void OnAnswerClicked_1();
	UFUNCTION() void OnAnswerClicked_2();
	UFUNCTION() void OnAnswerClicked_3();

	UFUNCTION() void OnSubmitClicked();

	// Logique commune
	void HandleFlagClicked(int32 FlagIndex);
	void HandleAnswerClicked(int32 AnswerIndex);
	void HighlightFlag(int32 FlagIndex, bool bHighlight);
	void RebuildConnectionLines();
	void ValidateAnswers();
	void ResetQuiz();

	// Helper pour obtenir la position centre d'un widget
	FVector2D GetWidgetCenterPosition(UWidget* Widget) const;
};