// FlagQuizWidget.cpp
#include "FlagQuizWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Rendering/DrawElements.h"
#include "Algo/RandomShuffle.h"

// ═══════════════════════════════════════════════════════════════════
//  NativeConstruct — appelé quand le widget est créé
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Remplir les arrays pour accès par index
	FlagButtons = { FlagButton_0, FlagButton_1, FlagButton_2, FlagButton_3 };
	AnswerButtons = { AnswerButton_0, AnswerButton_1, AnswerButton_2, AnswerButton_3 };
	FlagImages = { FlagImage_0, FlagImage_1, FlagImage_2, FlagImage_3 };
	AnswerTexts = { AnswerText_0, AnswerText_1, AnswerText_2, AnswerText_3 };

	SetupBindings();
	InitializeQuiz();
}

// ═══════════════════════════════════════════════════════════════════
//  Setup — Binding des événements OnClicked
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::SetupBindings()
{
	// Drapeaux
	if (FlagButton_0) FlagButton_0->OnClicked.AddDynamic(this, &UFlagQuizWidget::OnFlagClicked_0);
	if (FlagButton_1) FlagButton_1->OnClicked.AddDynamic(this, &UFlagQuizWidget::OnFlagClicked_1);
	if (FlagButton_2) FlagButton_2->OnClicked.AddDynamic(this, &UFlagQuizWidget::OnFlagClicked_2);
	if (FlagButton_3) FlagButton_3->OnClicked.AddDynamic(this, &UFlagQuizWidget::OnFlagClicked_3);

	// Réponses
	if (AnswerButton_0) AnswerButton_0->OnClicked.AddDynamic(this, &UFlagQuizWidget::OnAnswerClicked_0);
	if (AnswerButton_1) AnswerButton_1->OnClicked.AddDynamic(this, &UFlagQuizWidget::OnAnswerClicked_1);
	if (AnswerButton_2) AnswerButton_2->OnClicked.AddDynamic(this, &UFlagQuizWidget::OnAnswerClicked_2);
	if (AnswerButton_3) AnswerButton_3->OnClicked.AddDynamic(this, &UFlagQuizWidget::OnAnswerClicked_3);

	// Submit
	if (SubmitButton) SubmitButton->OnClicked.AddDynamic(this, &UFlagQuizWidget::OnSubmitClicked);
}

// ═══════════════════════════════════════════════════════════════════
//  InitializeQuiz — Remplir les drapeaux et mélanger les réponses
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::InitializeQuiz()
{
	// Reset
	SelectedFlagIndex = -1;
	bQuizSubmitted = false;
	PlayerLinks.Empty();
	ConnectionLines.Empty();

	if (FeedbackText)
	{
		FeedbackText->SetText(FText::GetEmpty());
	}

	// Si QuizPairs n'est pas rempli, utiliser des valeurs par défaut
	// (tu peux les set dans le Blueprint defaults ou ici)
	if (QuizPairs.Num() == 0)
	{
		FFlagQuizPair Pair;

		Pair.CountryName = TEXT("Chine");
		QuizPairs.Add(Pair);

		Pair.CountryName = TEXT("Haïti");
		QuizPairs.Add(Pair);

		Pair.CountryName = TEXT("Algérie");
		QuizPairs.Add(Pair);

		Pair.CountryName = TEXT("Philippines");
		QuizPairs.Add(Pair);
	}

	// Stocker les bonnes réponses : FlagIndex → nom du pays
	CorrectAnswers.Empty();
	for (int32 i = 0; i < QuizPairs.Num() && i < 4; i++)
	{
		CorrectAnswers.Add(i, QuizPairs[i].CountryName);

		// Appliquer la texture du drapeau si fournie
		if (QuizPairs[i].FlagTexture && FlagImages.IsValidIndex(i) && FlagImages[i])
		{
			FlagImages[i]->SetBrushFromTexture(QuizPairs[i].FlagTexture);
		}
	}

	// Mélanger les réponses
	ShuffleAnswers();
}

// ═══════════════════════════════════════════════════════════════════
//  ShuffleAnswers — Mélanger l'ordre des noms de pays à droite
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::ShuffleAnswers()
{
	TArray<FString> ShuffledNames;
	for (const auto& Pair : QuizPairs)
	{
		ShuffledNames.Add(Pair.CountryName);
	}

	// Mélange Fisher-Yates
	const int32 LastIndex = ShuffledNames.Num() - 1;
	for (int32 i = LastIndex; i > 0; i--)
	{
		const int32 j = FMath::RandRange(0, i);
		ShuffledNames.Swap(i, j);
	}

	// Appliquer aux textes
	for (int32 i = 0; i < ShuffledNames.Num() && i < AnswerTexts.Num(); i++)
	{
		if (AnswerTexts[i])
		{
			AnswerTexts[i]->SetText(FText::FromString(ShuffledNames[i]));
		}
	}
}

// ═══════════════════════════════════════════════════════════════════
//  Callbacks OnClicked — redirigent vers la logique commune
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::OnFlagClicked_0() { HandleFlagClicked(0); }
void UFlagQuizWidget::OnFlagClicked_1() { HandleFlagClicked(1); }
void UFlagQuizWidget::OnFlagClicked_2() { HandleFlagClicked(2); }
void UFlagQuizWidget::OnFlagClicked_3() { HandleFlagClicked(3); }

void UFlagQuizWidget::OnAnswerClicked_0() { HandleAnswerClicked(0); }
void UFlagQuizWidget::OnAnswerClicked_1() { HandleAnswerClicked(1); }
void UFlagQuizWidget::OnAnswerClicked_2() { HandleAnswerClicked(2); }
void UFlagQuizWidget::OnAnswerClicked_3() { HandleAnswerClicked(3); }

// ═══════════════════════════════════════════════════════════════════
//  HandleFlagClicked — Sélectionner/désélectionner un drapeau
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::HandleFlagClicked(int32 FlagIndex)
{
	// Bloquer si déjà soumis
	if (bQuizSubmitted) return;

	// Si on re-clique le même drapeau → désélectionner
	if (SelectedFlagIndex == FlagIndex)
	{
		HighlightFlag(SelectedFlagIndex, false);
		SelectedFlagIndex = -1;
		return;
	}

	// Désélectionner l'ancien si y'en avait un
	if (SelectedFlagIndex != -1)
	{
		HighlightFlag(SelectedFlagIndex, false);
	}

	// Si ce drapeau a déjà un lien, le retirer (pour permettre de rechanger)
	if (PlayerLinks.Contains(FlagIndex))
	{
		PlayerLinks.Remove(FlagIndex);
		RebuildConnectionLines();
	}

	// Sélectionner le nouveau
	SelectedFlagIndex = FlagIndex;
	HighlightFlag(FlagIndex, true);
}

// ═══════════════════════════════════════════════════════════════════
//  HandleAnswerClicked — Lier le drapeau sélectionné à cette réponse
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::HandleAnswerClicked(int32 AnswerIndex)
{
	// Rien à faire si pas de drapeau sélectionné ou déjà soumis
	if (SelectedFlagIndex == -1 || bQuizSubmitted) return;

	// Vérifier si cette réponse est déjà liée à un AUTRE drapeau → retirer l'ancien lien
	TArray<int32> KeysToRemove;
	for (const auto& Link : PlayerLinks)
	{
		if (Link.Value == AnswerIndex && Link.Key != SelectedFlagIndex)
		{
			KeysToRemove.Add(Link.Key);
		}
	}
	for (int32 Key : KeysToRemove)
	{
		PlayerLinks.Remove(Key);
	}

	// Créer le lien
	PlayerLinks.Add(SelectedFlagIndex, AnswerIndex);

	// Désélectionner le drapeau
	HighlightFlag(SelectedFlagIndex, false);
	SelectedFlagIndex = -1;

	// Reconstruire les lignes visuelles
	RebuildConnectionLines();
}

// ═══════════════════════════════════════════════════════════════════
//  HighlightFlag — Changer le style visuel d'un drapeau sélectionné
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::HighlightFlag(int32 FlagIndex, bool bHighlight)
{
	if (!FlagButtons.IsValidIndex(FlagIndex) || !FlagButtons[FlagIndex]) return;

	FButtonStyle Style = FlagButtons[FlagIndex]->GetStyle();

	if (bHighlight)
	{
		// Teinte jaune pour montrer la sélection
		Style.Normal.TintColor = FSlateColor(FLinearColor(1.0f, 0.8f, 0.0f, 1.0f));
	}
	else
	{
		// Retour à la normale
		Style.Normal.TintColor = FSlateColor(FLinearColor::White);
	}

	FlagButtons[FlagIndex]->SetStyle(Style);
}

// ═══════════════════════════════════════════════════════════════════
//  RebuildConnectionLines — Reconstruire la liste des lignes
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::RebuildConnectionLines()
{
	ConnectionLines.Empty();

	for (const auto& Link : PlayerLinks)
	{
		FQuizConnectionLine Line;
		Line.FlagIndex = Link.Key;
		Line.AnswerIndex = Link.Value;
		Line.LineColor = FLinearColor(1.0f, 0.9f, 0.3f, 1.0f); // Jaune doré
		ConnectionLines.Add(Line);
	}

	// Forcer le redraw
	Invalidate(EInvalidateWidgetReason::Paint);
}

// ═══════════════════════════════════════════════════════════════════
//  OnSubmitClicked — Valider les réponses du joueur
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::OnSubmitClicked()
{
	if (bQuizSubmitted) return;

	// Vérifier que tout est lié
	if (PlayerLinks.Num() < QuizPairs.Num())
	{
		if (FeedbackText)
		{
			FeedbackText->SetText(FText::FromString(
				TEXT("Associez tous les drapeaux avant de soumettre!")
			));
		}
		return;
	}

	bQuizSubmitted = true;
	ValidateAnswers();
}

// ═══════════════════════════════════════════════════════════════════
//  ValidateAnswers — Comparer les liens du joueur aux bonnes réponses
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::ValidateAnswers()
{
	int32 CorrectCount = 0;

	// Reconstruire les lignes avec les couleurs de feedback
	ConnectionLines.Empty();

	for (const auto& Link : PlayerLinks)
	{
		int32 FlagIdx = Link.Key;
		int32 AnsIdx = Link.Value;

		FQuizConnectionLine Line;
		Line.FlagIndex = FlagIdx;
		Line.AnswerIndex = AnsIdx;

		// Récupérer le texte de la réponse choisie
		FString ChosenAnswer;
		if (AnswerTexts.IsValidIndex(AnsIdx) && AnswerTexts[AnsIdx])
		{
			ChosenAnswer = AnswerTexts[AnsIdx]->GetText().ToString();
		}

		// Comparer avec la bonne réponse
		const FString* CorrectName = CorrectAnswers.Find(FlagIdx);
		if (CorrectName && ChosenAnswer.Equals(*CorrectName))
		{
			Line.LineColor = FLinearColor::Green;
			CorrectCount++;
		}
		else
		{
			Line.LineColor = FLinearColor::Red;
		}

		ConnectionLines.Add(Line);
	}

	// Afficher le feedback
	if (FeedbackText)
	{
		if (CorrectCount == QuizPairs.Num())
		{
			FeedbackText->SetText(FText::FromString(
				FString::Printf(TEXT("Bravo! %d/%d — Tout est correct!"), CorrectCount, QuizPairs.Num())
			));
		}
		else
		{
			FeedbackText->SetText(FText::FromString(
				FString::Printf(TEXT("%d/%d bonnes réponses. Réessaie!"), CorrectCount, QuizPairs.Num())
			));
		}
	}

	// Désactiver les boutons
	for (UButton* Btn : FlagButtons)
	{
		if (Btn) Btn->SetIsEnabled(false);
	}
	for (UButton* Btn : AnswerButtons)
	{
		if (Btn) Btn->SetIsEnabled(false);
	}

	// Forcer le redraw des lignes colorées
	Invalidate(EInvalidateWidgetReason::Paint);
}

// ═══════════════════════════════════════════════════════════════════
//  NativePaint — Dessiner les lignes de connexion
// ═══════════════════════════════════════════════════════════════════
int32 UFlagQuizWidget::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled
) const
{
	// D'abord dessiner le widget normalement
	int32 Result = Super::NativePaint(
		Args, AllottedGeometry, MyCullingRect,
		OutDrawElements, LayerId, InWidgetStyle, bParentEnabled
	);

	// Ensuite dessiner les lignes par-dessus
	const int32 LineLayerId = LayerId + 1;

	for (const FQuizConnectionLine& Line : ConnectionLines)
	{
		if (!FlagButtons.IsValidIndex(Line.FlagIndex) || !AnswerButtons.IsValidIndex(Line.AnswerIndex))
			continue;

		UButton* FlagBtn = FlagButtons[Line.FlagIndex];
		UButton* AnsBtn = AnswerButtons[Line.AnswerIndex];

		if (!FlagBtn || !AnsBtn) continue;

		// Obtenir les positions en coordonnées locales du widget
		const FGeometry& FlagGeom = FlagBtn->GetCachedGeometry();
		const FGeometry& AnsGeom = AnsBtn->GetCachedGeometry();

		// Centre du bouton drapeau (côté droit)
		FVector2D FlagAbsPos = FlagGeom.GetAbsolutePosition();
		FVector2D FlagAbsSize = FlagGeom.GetAbsoluteSize();
		FVector2D FlagCenter(
			FlagAbsPos.X + FlagAbsSize.X, // Bord droit du drapeau
			FlagAbsPos.Y + FlagAbsSize.Y * 0.5f
		);

		// Centre du bouton réponse (côté gauche)
		FVector2D AnsAbsPos = AnsGeom.GetAbsolutePosition();
		FVector2D AnsAbsSize = AnsGeom.GetAbsoluteSize();
		FVector2D AnsCenter(
			AnsAbsPos.X, // Bord gauche de la réponse
			AnsAbsPos.Y + AnsAbsSize.Y * 0.5f
		);

		// Convertir en coordonnées locales du widget parent
		FVector2D LocalStart = AllottedGeometry.AbsoluteToLocal(FlagCenter);
		FVector2D LocalEnd = AllottedGeometry.AbsoluteToLocal(AnsCenter);

		// Dessiner la ligne
		TArray<FVector2D> LinePoints;
		LinePoints.Add(LocalStart);
		LinePoints.Add(LocalEnd);

		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LineLayerId,
			AllottedGeometry.ToPaintGeometry(),
			LinePoints,
			ESlateDrawEffect::None,
			Line.LineColor,
			true,       // bAntialias
			3.0f        // Épaisseur
		);
	}

	return Result;
}

// ═══════════════════════════════════════════════════════════════════
//  ResetQuiz — Réinitialiser le quiz pour rejouer
// ═══════════════════════════════════════════════════════════════════
void UFlagQuizWidget::ResetQuiz()
{
	// Réactiver les boutons
	for (UButton* Btn : FlagButtons)
	{
		if (Btn) Btn->SetIsEnabled(true);
	}
	for (UButton* Btn : AnswerButtons)
	{
		if (Btn) Btn->SetIsEnabled(true);
	}

	// Réinitialiser le quiz (remélange les réponses)
	InitializeQuiz();
}

// ═══════════════════════════════════════════════════════════════════
//  GetWidgetCenterPosition — Helper (pas utilisé dans NativePaint
//  mais utile si tu veux la position depuis le gameplay code)
// ═══════════════════════════════════════════════════════════════════
FVector2D UFlagQuizWidget::GetWidgetCenterPosition(UWidget* Widget) const
{
	if (!Widget) return FVector2D::ZeroVector;

	const FGeometry& Geom = Widget->GetCachedGeometry();
	FVector2D AbsPos = Geom.GetAbsolutePosition();
	FVector2D AbsSize = Geom.GetAbsoluteSize();

	return AbsPos + AbsSize * 0.5f;
}