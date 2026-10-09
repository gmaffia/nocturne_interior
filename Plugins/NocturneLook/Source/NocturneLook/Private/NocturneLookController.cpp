#include "NocturneLookController.h"

#include "Components/PostProcessComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/Scene.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Runtime/Launch/Resources/Version.h"

ANocturneLookController::ANocturneLookController()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRoot);

    PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("NocturnePostProcess"));
    PostProcess->SetupAttachment(SceneRoot);
    PostProcess->bUnbound = true;
    PostProcess->BlendWeight = 1.f;
    PostProcess->Priority = 100.f;
}

void ANocturneLookController::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    BuildSettings();
    ApplyActiveSettings();
}

void ANocturneLookController::BeginPlay()
{
    Super::BeginPlay();
    BuildSettings();
    ApplyActiveSettings();
}

void ANocturneLookController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bEnableRuntimeControls)
    {
        HandleRuntimeControls();
    }

    if (bShowRuntimePanel)
    {
        DrawRuntimePanel();
    }
}

void ANocturneLookController::BuildSettings()
{
    LookSettings = FPostProcessSettings();
    LookSettings.bOverride_AutoExposureMethod = true;
    LookSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
    LookSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
    LookSettings.AutoExposureApplyPhysicalCameraExposure = false;
    LookSettings.bOverride_AutoExposureBias = true;
    LookSettings.AutoExposureBias = ExposureBias;
    LookSettings.bOverride_FilmGrainIntensity = true;
    LookSettings.FilmGrainIntensity = FilmGrainIntensity;
    LookSettings.bOverride_FilmGrainIntensityShadows = true;
    LookSettings.FilmGrainIntensityShadows = 1.f;
    LookSettings.bOverride_FilmGrainIntensityMidtones = true;
    LookSettings.FilmGrainIntensityMidtones = .45f;
    LookSettings.bOverride_FilmGrainIntensityHighlights = true;
    LookSettings.FilmGrainIntensityHighlights = .1f;
#if ENGINE_MAJOR_VERSION > 5 || ENGINE_MINOR_VERSION >= 5
    LookSettings.bOverride_FilmGrainScale = true;
    LookSettings.FilmGrainScale = FVector2f(1.35f, 1.35f);
#else
    LookSettings.bOverride_FilmGrainTexelSize = true;
    LookSettings.FilmGrainTexelSize = 1.35f;
#endif
    LookSettings.bOverride_VignetteIntensity = true;
    LookSettings.VignetteIntensity = VignetteIntensity;
    LookSettings.bOverride_SceneFringeIntensity = true;
    LookSettings.SceneFringeIntensity = ChromaticAberrationIntensity;
    LookSettings.bOverride_ColorSaturation = true;
    LookSettings.ColorSaturation = FVector4(Saturation, Saturation, Saturation, 1.f);
    LookSettings.bOverride_ColorContrast = true;
    LookSettings.ColorContrast = FVector4(Contrast, Contrast, Contrast, 1.f);

}

void ANocturneLookController::ApplyActiveSettings()
{
    const bool bWasRegistered = PostProcess->IsRegistered();
    if (bWasRegistered)
    {
        PostProcess->UnregisterComponent();
    }

    PostProcess->Settings = LookSettings;
    // A small non-zero neutral weight keeps Unreal from pruning the component
    // from the world's post-process list, allowing deterministic reactivation.
    PostProcess->BlendWeight = bLookEnabled ? 1.f : KINDA_SMALL_NUMBER;
    PostProcess->SetActive(true, true);

    if (bWasRegistered)
    {
        PostProcess->RegisterComponent();
    }
    PostProcess->MarkRenderStateDirty();
}

void ANocturneLookController::AdjustExposure(float Delta)
{
    ExposureBias = FMath::Clamp(ExposureBias + Delta, -3.f, 5.f);
    BuildSettings();
    ApplyActiveSettings();
}

void ANocturneLookController::AdjustFilmGrain(float Delta)
{
    FilmGrainIntensity = FMath::Clamp(FilmGrainIntensity + Delta, 0.f, 1.f);
    BuildSettings();
    ApplyActiveSettings();
}

void ANocturneLookController::AdjustVignette(float Delta)
{
    VignetteIntensity = FMath::Clamp(VignetteIntensity + Delta, 0.f, 1.f);
    BuildSettings();
    ApplyActiveSettings();
}

void ANocturneLookController::AdjustChromaticAberration(float Delta)
{
    ChromaticAberrationIntensity = FMath::Clamp(ChromaticAberrationIntensity + Delta, 0.f, 5.f);
    BuildSettings();
    ApplyActiveSettings();
}

void ANocturneLookController::ToggleLookEnabled()
{
    bLookEnabled = !bLookEnabled;
    ApplyActiveSettings();

    const FString StateText = FString::Printf(
        TEXT("NOCTURNE LOOK: %s  |  BLEND: %.0f"),
        bLookEnabled ? TEXT("ON") : TEXT("OFF"),
        PostProcess->BlendWeight);
    UE_LOG(LogTemp, Display, TEXT("%s"), *StateText);
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            8127,
            2.5f,
            bLookEnabled ? FColor(100, 220, 140) : FColor(230, 170, 80),
            StateText);
    }
}

void ANocturneLookController::ResetLook()
{
    bLookEnabled = true;
    ExposureBias = -1.25f;
    FilmGrainIntensity = 0.65f;
    VignetteIntensity = 0.45f;
    ChromaticAberrationIntensity = 0.45f;
    Saturation = 0.72f;
    Contrast = 1.06f;
    BuildSettings();
    ApplyActiveSettings();
}

float ANocturneLookController::GetExposure() const
{
    return ExposureBias;
}

float ANocturneLookController::GetFilmGrain() const
{
    return FilmGrainIntensity;
}

float ANocturneLookController::GetVignette() const
{
    return VignetteIntensity;
}

float ANocturneLookController::GetChromaticAberration() const
{
    return ChromaticAberrationIntensity;
}

void ANocturneLookController::HandleRuntimeControls()
{
    APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PlayerController)
    {
        return;
    }

    if (PlayerController->WasInputKeyJustPressed(EKeys::Z)) bShowRuntimePanel = !bShowRuntimePanel;
    if (PlayerController->WasInputKeyJustPressed(EKeys::X)) ToggleLookEnabled();
    if (PlayerController->WasInputKeyJustPressed(EKeys::R)) ResetLook();
    if (PlayerController->WasInputKeyJustPressed(EKeys::One)) AdjustExposure(-0.25f);
    if (PlayerController->WasInputKeyJustPressed(EKeys::Two)) AdjustExposure(0.25f);
    if (PlayerController->WasInputKeyJustPressed(EKeys::Three)) AdjustFilmGrain(-0.05f);
    if (PlayerController->WasInputKeyJustPressed(EKeys::Four)) AdjustFilmGrain(0.05f);
    if (PlayerController->WasInputKeyJustPressed(EKeys::Five)) AdjustVignette(-0.05f);
    if (PlayerController->WasInputKeyJustPressed(EKeys::Six)) AdjustVignette(0.05f);
    if (PlayerController->WasInputKeyJustPressed(EKeys::Seven)) AdjustChromaticAberration(-0.10f);
    if (PlayerController->WasInputKeyJustPressed(EKeys::Eight)) AdjustChromaticAberration(0.10f);
}

void ANocturneLookController::DrawRuntimePanel() const
{
    if (!GEngine)
    {
        return;
    }

    const FColor TextColor = bLookEnabled ? FColor(180, 220, 190) : FColor(230, 170, 80);
    GEngine->AddOnScreenDebugMessage(9200, 0.05f, TextColor, TEXT("NOCTURNE LOOK  [Z: hide] [X: toggle] [R: reset]"));
    GEngine->AddOnScreenDebugMessage(9201, 0.05f, TextColor, FString::Printf(TEXT("[1 / 2] Exposure Bias: %.2f"), ExposureBias));
    GEngine->AddOnScreenDebugMessage(9202, 0.05f, TextColor, FString::Printf(TEXT("[3 / 4] Film Grain: %.2f"), FilmGrainIntensity));
    GEngine->AddOnScreenDebugMessage(9203, 0.05f, TextColor, FString::Printf(TEXT("[5 / 6] Vignette: %.2f"), VignetteIntensity));
    GEngine->AddOnScreenDebugMessage(9204, 0.05f, TextColor, FString::Printf(TEXT("[7 / 8] Chromatic Aberration: %.2f"), ChromaticAberrationIntensity));
}
