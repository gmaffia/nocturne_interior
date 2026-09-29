#include "NocturnePlayerHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "GameFramework/PlayerController.h"
#include "NocturnePlayerCharacter.h"

void ANocturnePlayerHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas || !PlayerOwner)
    {
        return;
    }

    const ANocturnePlayerCharacter* PlayerCharacter =
        Cast<ANocturnePlayerCharacter>(PlayerOwner->GetPawn());
    if (!PlayerCharacter)
    {
        return;
    }

    UFont* Font = GEngine ? GEngine->GetSmallFont() : nullptr;
    if (!Font)
    {
        return;
    }

    constexpr float RightMargin = 28.0f;
    constexpr float TopMargin = 28.0f;
    constexpr float LineHeight = 22.0f;
    constexpr float HorizontalPadding = 14.0f;
    constexpr float VerticalPadding = 10.0f;

    if (PlayerCharacter->ShouldShowControlHints())
    {
    const TArray<FString> Lines = {
        FString::Printf(TEXT("Stance: %s"), *PlayerCharacter->GetCurrentStanceDisplayName().ToString()),
        FString::Printf(TEXT("Graphics: %s"), *PlayerCharacter->GetGraphicsQualityDisplayName().ToString()),
        TEXT("E  Raise stance"),
        TEXT("C  Lower stance"),
        TEXT("G  Cycle graphics and lighting quality"),
        TEXT("WASD  Move"),
        TEXT("Mouse  Look"),
        TEXT("H  Hide controls")
    };

    float MaximumTextWidth = 0.0f;
    for (const FString& Line : Lines)
    {
        float TextWidth = 0.0f;
        float TextHeight = 0.0f;
        GetTextSize(Line, TextWidth, TextHeight, Font);
        MaximumTextWidth = FMath::Max(MaximumTextWidth, TextWidth);
    }

    const float PanelWidth = MaximumTextWidth + HorizontalPadding * 2.0f;
    const float PanelHeight = Lines.Num() * LineHeight + VerticalPadding * 2.0f;
    const float PanelX = Canvas->ClipX - RightMargin - PanelWidth;
    const float PanelY = TopMargin;

    DrawRect(FLinearColor(0.01f, 0.01f, 0.015f, 0.72f), PanelX, PanelY, PanelWidth, PanelHeight);

    for (int32 Index = 0; Index < Lines.Num(); ++Index)
    {
        const FLinearColor TextColor = Index == 0
            ? FLinearColor(0.72f, 0.82f, 1.0f, 1.0f)
            : FLinearColor(0.88f, 0.88f, 0.9f, 1.0f);
        DrawText(
            Lines[Index],
            TextColor,
            PanelX + HorizontalPadding,
            PanelY + VerticalPadding + Index * LineHeight,
            Font,
            1.0f,
            false);
    }
    }

    if (!PlayerCharacter->ShouldShowGraphicsFeaturePanel())
    {
        return;
    }

    const auto StateLabel = [](bool bEnabled)
    {
        return bEnabled ? TEXT("ON") : TEXT("OFF");
    };
    const TArray<FString> GraphicsLines = {
        TEXT("GRAPHICS FEATURES  [P: hide]"),
        FString::Printf(
            TEXT("[M] Direct lighting: %s"),
            PlayerCharacter->IsMegaLightsEnabled() ? TEXT("MegaLights") : TEXT("Deferred")),
        FString::Printf(
            TEXT("[J] Lumen tracing: %s"),
            PlayerCharacter->IsLumenHardwareRayTracingEnabled() ? TEXT("Hardware") : TEXT("Software")),
        FString::Printf(
            TEXT("[V] Volumetric Fog: %s"),
            StateLabel(PlayerCharacter->IsVolumetricFogEnabled())),
        FString::Printf(
            TEXT("[T] Television shadows: %s"),
            StateLabel(PlayerCharacter->AreTelevisionShadowsEnabled())),
        FString::Printf(
            TEXT("[Y] Strobe shadows: %s"),
            StateLabel(PlayerCharacter->AreStrobeShadowsEnabled())),
        FString::Printf(
            TEXT("[B] Strobe beam: %s"),
            PlayerCharacter->IsStrobeVolumetricBeamEnabled() ? TEXT("Volumetric") : TEXT("Surface Only")),
        TEXT("Virtual Shadow Maps: ON (restart required)"),
        TEXT("Substrate: ON (restart required)")
    };

    float GraphicsMaximumTextWidth = 0.0f;
    for (const FString& Line : GraphicsLines)
    {
        float TextWidth = 0.0f;
        float TextHeight = 0.0f;
        GetTextSize(Line, TextWidth, TextHeight, Font);
        GraphicsMaximumTextWidth = FMath::Max(GraphicsMaximumTextWidth, TextWidth);
    }

    const float GraphicsPanelWidth = GraphicsMaximumTextWidth + HorizontalPadding * 2.0f;
    const float GraphicsPanelHeight = GraphicsLines.Num() * LineHeight + VerticalPadding * 2.0f;
    const float GraphicsPanelX = 28.0f;
    const float GraphicsPanelY = Canvas->ClipY - 28.0f - GraphicsPanelHeight;
    DrawRect(
        FLinearColor(0.01f, 0.01f, 0.015f, 0.78f),
        GraphicsPanelX,
        GraphicsPanelY,
        GraphicsPanelWidth,
        GraphicsPanelHeight);

    for (int32 Index = 0; Index < GraphicsLines.Num(); ++Index)
    {
        const FLinearColor TextColor = Index == 0
            ? FLinearColor(0.72f, 0.82f, 1.0f, 1.0f)
            : FLinearColor(0.88f, 0.88f, 0.9f, 1.0f);
        DrawText(
            GraphicsLines[Index],
            TextColor,
            GraphicsPanelX + HorizontalPadding,
            GraphicsPanelY + VerticalPadding + Index * LineHeight,
            Font,
            1.0f,
            false);
    }
}
