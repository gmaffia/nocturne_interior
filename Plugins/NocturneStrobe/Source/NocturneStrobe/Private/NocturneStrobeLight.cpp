#include "NocturneStrobeLight.h"

#include "Components/AudioComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"

ANocturneStrobeLight::ANocturneStrobeLight()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    SweepPivot = CreateDefaultSubobject<USceneComponent>(TEXT("SweepPivot"));
    SweepPivot->SetupAttachment(SceneRoot);

    StrobeLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("StrobeLight"));
    StrobeLight->SetupAttachment(SweepPivot);
    StrobeLight->SetMobility(EComponentMobility::Movable);

    DirectionalAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("DirectionalAudio"));
    DirectionalAudio->SetupAttachment(SweepPivot);
    DirectionalAudio->SetMobility(EComponentMobility::Movable);
    DirectionalAudio->bAutoActivate = false;
}

void ANocturneStrobeLight::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    RefreshSettings();
}

void ANocturneStrobeLight::BeginPlay()
{
    Super::BeginPlay();

    bRuntimeEnabled = bStartEnabled;
    bControlsPanelVisible = bShowControlsPanelAtStart;
    RuntimeTime = InitialPhase / FMath::Max(Frequency, 0.1f);
    CurrentIntensity = MinimumIntensity;
    RandomStream.Initialize(RandomSeed == 0 ? GetUniqueID() : RandomSeed);
    ScheduleNextIrregularPulse();
    RefreshSettings();
    UpdateAudioPlaybackState();

    if (bEnableKeyboardToggle || bEnableControlsPanelToggle)
    {
        if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
        {
            EnableInput(PlayerController);
            if (InputComponent)
            {
                if (bEnableKeyboardToggle)
                {
                    InputComponent->BindKey(ToggleKey, IE_Pressed, this, &ANocturneStrobeLight::ToggleStrobe);
                }
                if (bEnableControlsPanelToggle)
                {
                    InputComponent->BindKey(ControlsPanelKey, IE_Pressed, this, &ANocturneStrobeLight::ToggleControlsPanel);
                }
            }

        }
    }

    EnsureHUDRegistration();
}

void ANocturneStrobeLight::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (RegisteredHUD.IsValid())
    {
        RegisteredHUD->RemovePostRenderedActor(this);
        RegisteredHUD.Reset();
    }

    if (DirectionalAudio)
    {
        DirectionalAudio->Stop();
    }

    Super::EndPlay(EndPlayReason);
}

void ANocturneStrobeLight::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    RuntimeTime += DeltaSeconds;
    ManualFlashRemaining = FMath::Max(ManualFlashRemaining - DeltaSeconds, 0.0f);
    UpdateLightColor();
    UpdateLight(DeltaSeconds);
    UpdateSweep();
    EnsureHUDRegistration();

    if (bLoopAudio)
    {
        UpdateAudioPlaybackState();
    }
}

void ANocturneStrobeLight::SetStrobeEnabled(bool bEnabled)
{
    bRuntimeEnabled = bEnabled;
    UpdateAudioPlaybackState();
}

void ANocturneStrobeLight::ToggleStrobe()
{
    SetStrobeEnabled(!bRuntimeEnabled);
}

void ANocturneStrobeLight::ToggleControlsPanel()
{
    bControlsPanelVisible = !bControlsPanelVisible;
}

void ANocturneStrobeLight::TriggerFlash(float Duration)
{
    ManualFlashRemaining = FMath::Max(Duration, 0.0f);
}

void ANocturneStrobeLight::RefreshSettings()
{
    StrobeLight->SetLightColor(
        ColorMode == ENocturneStrobeColorMode::CosmicViolet
            ? CosmicVioletColor
            : LightColor);
    StrobeLight->SetUseTemperature(bUseTemperature);
    StrobeLight->SetTemperature(Temperature);
    StrobeLight->SetAttenuationRadius(AttenuationRadius);
    StrobeLight->SetInnerConeAngle(FMath::Min(InnerConeAngle, OuterConeAngle));
    StrobeLight->SetOuterConeAngle(OuterConeAngle);
    StrobeLight->SetSourceRadius(SourceRadius);
    StrobeLight->SetVolumetricScatteringIntensity(VolumetricScatteringIntensity);
    StrobeLight->SetCastShadows(bCastShadows);
    StrobeLight->SetCastVolumetricShadow(bCastVolumetricShadow);
    SweepPivot->SetRelativeRotation(BaseLightRotation);
    RefreshAudioSettings();

    if (!GetWorld() || !GetWorld()->IsGameWorld())
    {
        StrobeLight->SetIntensity(bStartEnabled ? MaximumIntensity : MinimumIntensity);
    }
}

void ANocturneStrobeLight::RefreshAudioSettings()
{
    if (!DirectionalAudio)
    {
        return;
    }

    DirectionalAudio->SetSound(SoundSource);
    DirectionalAudio->SetVolumeMultiplier(AudioVolume);
    DirectionalAudio->SetPitchMultiplier(AudioPitch);
    DirectionalAudio->bOverrideAttenuation = true;

    FSoundAttenuationSettings Settings;
    Settings.bAttenuate = true;
    Settings.bSpatialize = true;
    Settings.bEnableOcclusion = bEnableAudioOcclusion;
    Settings.AttenuationShape = EAttenuationShape::Cone;
    Settings.AttenuationShapeExtents = FVector(
        AudioInnerRadius,
        FMath::Clamp(AudioInnerConeAngle, 0.0f, 180.0f),
        FMath::Max(AudioOuterConeAngle - AudioInnerConeAngle, 0.0f));
    Settings.FalloffDistance = AudioFalloffDistance;
    Settings.ConeOffset = AudioConeOffset;
    Settings.ConeSphereRadius = AudioRearBleedRadius;
    Settings.ConeSphereFalloffDistance = AudioRearBleedFalloffDistance;
    DirectionalAudio->AdjustAttenuation(Settings);
}

void ANocturneStrobeLight::UpdateAudioPlaybackState()
{
    if (!DirectionalAudio)
    {
        return;
    }

    if (!SoundSource || !bAutoPlayAudio)
    {
        DirectionalAudio->Stop();
        return;
    }

    const bool bShouldPlay = !bAudioFollowsStrobeState || bRuntimeEnabled;
    if (bShouldPlay)
    {
        if (!DirectionalAudio->IsPlaying())
        {
            DirectionalAudio->Play();
        }
        else
        {
            DirectionalAudio->SetPaused(false);
        }
    }
    else
    {
        DirectionalAudio->SetPaused(true);
    }
}

void ANocturneStrobeLight::EnsureHUDRegistration()
{
    if (RegisteredHUD.IsValid() || !GetWorld())
    {
        return;
    }

    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        if (AHUD* HUD = PlayerController->GetHUD())
        {
            HUD->bShowOverlays = true;
            HUD->AddPostRenderedActor(this);
            RegisteredHUD = HUD;
        }
    }
}

void ANocturneStrobeLight::PostRenderFor(
    APlayerController* PlayerController,
    UCanvas* Canvas,
    FVector CameraPosition,
    FVector CameraDirection)
{
    Super::PostRenderFor(PlayerController, Canvas, CameraPosition, CameraDirection);

    if (!bControlsPanelVisible || !Canvas || !GEngine)
    {
        return;
    }

    UFont* Font = GEngine->GetSmallFont();
    if (!Font)
    {
        return;
    }

    const FString MotionName = MotionMode == ENocturneStrobeMotionMode::ContinuousRotation
        ? TEXT("Axial Spin")
        : TEXT("Oscillate");
    const FString ColorModeName = ColorMode == ENocturneStrobeColorMode::CosmicViolet
        ? TEXT("Cosmic Violet")
        : TEXT("Fixed");
    const FString AudioState = SoundSource
        ? (DirectionalAudio && DirectionalAudio->IsPlaying() ? TEXT("PLAYING") : TEXT("PAUSED"))
        : TEXT("NOT ASSIGNED");
    const TArray<FString> Lines = {
        FString::Printf(TEXT("Nocturne Strobe: %s"), bRuntimeEnabled ? TEXT("ON") : TEXT("OFF")),
        FString::Printf(TEXT("%s  Toggle strobe"), *ToggleKey.GetDisplayName().ToString()),
        FString::Printf(TEXT("%s  Hide strobe controls"), *ControlsPanelKey.GetDisplayName().ToString()),
        FString::Printf(TEXT("Motion: %s"), *MotionName),
        FString::Printf(TEXT("Color: %s"), *ColorModeName),
        FString::Printf(TEXT("Intensity: %.0f"), MaximumIntensity),
        FString::Printf(TEXT("Directional audio: %s"), *AudioState)
    };

    constexpr float RightMargin = 28.0f;
    constexpr float PlayerControlsTop = 28.0f;
    constexpr float PlayerControlsHeight = 152.0f;
    constexpr float PanelGap = 12.0f;
    constexpr float LineHeight = 22.0f;
    constexpr float HorizontalPadding = 14.0f;
    constexpr float VerticalPadding = 10.0f;

    float MaximumTextWidth = 0.0f;
    for (const FString& Line : Lines)
    {
        float TextWidth = 0.0f;
        float TextHeight = 0.0f;
        Canvas->StrLen(Font, Line, TextWidth, TextHeight);
        MaximumTextWidth = FMath::Max(MaximumTextWidth, TextWidth);
    }

    const FVector2D PanelSize(
        MaximumTextWidth + HorizontalPadding * 2.0f,
        Lines.Num() * LineHeight + VerticalPadding * 2.0f);
    const float PanelX = Canvas->ClipX - RightMargin - PanelSize.X;
    const float PanelY = PlayerControlsTop + PlayerControlsHeight + PanelGap;
    FCanvasTileItem Background(
        FVector2D(PanelX, PanelY),
        PanelSize,
        FLinearColor(0.01f, 0.01f, 0.015f, 0.72f));
    Background.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(Background);

    for (int32 Index = 0; Index < Lines.Num(); ++Index)
    {
        const FLinearColor TextColor = Index == 0
            ? FLinearColor(0.72f, 0.82f, 1.0f, 1.0f)
            : FLinearColor(0.88f, 0.88f, 0.9f, 1.0f);
        FCanvasTextItem TextItem(
            FVector2D(
                PanelX + HorizontalPadding,
                PanelY + VerticalPadding + Index * LineHeight),
            FText::FromString(Lines[Index]),
            Font,
            TextColor);
        TextItem.EnableShadow(FLinearColor::Black);
        Canvas->DrawItem(TextItem);
    }
}

void ANocturneStrobeLight::UpdateLightColor()
{
    if (ColorMode != ENocturneStrobeColorMode::CosmicViolet)
    {
        StrobeLight->SetLightColor(LightColor);
        return;
    }

    const float Speed = FMath::Max(CosmicColorCycleSpeed, 0.01f);
    const float PrimaryWave = 0.5f + 0.5f * FMath::Sin(RuntimeTime * Speed * UE_TWO_PI);
    const float SecondaryWave = 0.5f + 0.5f * FMath::Sin(
        RuntimeTime * Speed * UE_TWO_PI * 0.37f + 1.73f);
    const float Noise = 0.5f + 0.5f * FMath::PerlinNoise1D(
        RuntimeTime * Speed * 0.63f + RandomSeed * 0.013f);
    const float ColorIrregularity = FMath::Clamp(CosmicColorIrregularity, 0.0f, 1.0f);
    const float VioletToMagenta = FMath::Lerp(PrimaryWave, Noise, ColorIrregularity);
    const float PurpleAccent = SecondaryWave * (0.18f + ColorIrregularity * 0.42f);

    FLinearColor CosmicColor = FMath::Lerp(
        CosmicVioletColor,
        CosmicMagentaColor,
        VioletToMagenta);
    CosmicColor = FMath::Lerp(CosmicColor, CosmicPurpleColor, PurpleAccent);
    CosmicColor.A = 1.0f;
    StrobeLight->SetLightColor(CosmicColor);
}

bool ANocturneStrobeLight::EvaluateRegularPulse() const
{
    const float CycleDuration = 1.0f / FMath::Max(Frequency, 0.1f);
    const float CycleTime = FMath::Fmod(RuntimeTime, CycleDuration);
    return CycleTime < CycleDuration * FMath::Clamp(DutyCycle, 0.01f, 0.95f);
}

bool ANocturneStrobeLight::EvaluateDoubleFlashPulse() const
{
    const float CycleDuration = 1.0f / FMath::Max(Frequency, 0.1f);
    const float CycleTime = FMath::Fmod(RuntimeTime, CycleDuration);
    const float TotalPulseTime = CycleDuration * FMath::Clamp(DutyCycle, 0.01f, 0.8f);
    const float PulseDuration = TotalPulseTime * 0.5f;
    const float SecondPulseStart = PulseDuration + FMath::Min(DoubleFlashGap, CycleDuration * 0.5f);
    return CycleTime < PulseDuration
        || (CycleTime >= SecondPulseStart && CycleTime < SecondPulseStart + PulseDuration);
}

bool ANocturneStrobeLight::EvaluateIrregularPulse(float DeltaSeconds)
{
    IrregularPulseRemaining = FMath::Max(IrregularPulseRemaining - DeltaSeconds, 0.0f);
    TimeUntilIrregularPulse -= DeltaSeconds;
    if (TimeUntilIrregularPulse <= 0.0f)
    {
        const float CycleDuration = 1.0f / FMath::Max(Frequency, 0.1f);
        IrregularPulseRemaining = CycleDuration * FMath::Clamp(DutyCycle, 0.01f, 0.95f);
        ScheduleNextIrregularPulse();
    }
    return IrregularPulseRemaining > 0.0f;
}

void ANocturneStrobeLight::UpdateLight(float DeltaSeconds)
{
    bool bPulseActive = ManualFlashRemaining > 0.0f;
    if (bRuntimeEnabled && !bPulseActive)
    {
        switch (Pattern)
        {
        case ENocturneStrobePattern::Regular:
            bPulseActive = EvaluateRegularPulse();
            break;
        case ENocturneStrobePattern::Irregular:
            bPulseActive = EvaluateIrregularPulse(DeltaSeconds);
            break;
        case ENocturneStrobePattern::DoubleFlash:
        default:
            bPulseActive = EvaluateDoubleFlashPulse();
            break;
        }
    }

    const float TargetIntensity = bPulseActive
        ? MaximumIntensity
        : (bRuntimeEnabled ? MinimumIntensity : 0.0f);
    CurrentIntensity = FMath::FInterpTo(
        CurrentIntensity,
        TargetIntensity,
        DeltaSeconds,
        FMath::Max(IntensityResponseSpeed, 1.0f));
    StrobeLight->SetIntensity(CurrentIntensity);
}

void ANocturneStrobeLight::UpdateSweep()
{
    if (!bEnableSweep)
    {
        SweepPivot->SetRelativeRotation(BaseLightRotation);
        return;
    }

    if (MotionMode == ENocturneStrobeMotionMode::ContinuousRotation)
    {
        SweepPivot->SetRelativeRotation(FRotator(
            BaseLightRotation.Pitch + RuntimeTime * ContinuousPitchSpeed,
            BaseLightRotation.Yaw + RuntimeTime * ContinuousYawSpeed,
            BaseLightRotation.Roll + RuntimeTime * ContinuousRollSpeed));
        return;
    }

    const float Phase = FMath::DegreesToRadians(SweepPhaseDegrees);
    const float SweepAngle = RuntimeTime * SweepFrequency * UE_TWO_PI + Phase;
    const float YawOffset = FMath::Sin(SweepAngle) * YawAmplitude;
    const float PitchOffset = FMath::Sin(SweepAngle * 0.73f + 1.17f) * PitchAmplitude;
    SweepPivot->SetRelativeRotation(FRotator(
        BaseLightRotation.Pitch + PitchOffset,
        BaseLightRotation.Yaw + YawOffset,
        BaseLightRotation.Roll));
}

void ANocturneStrobeLight::ScheduleNextIrregularPulse()
{
    const float CycleDuration = 1.0f / FMath::Max(Frequency, 0.1f);
    const float Variation = FMath::Clamp(Irregularity, 0.0f, 1.0f);
    TimeUntilIrregularPulse = CycleDuration * RandomStream.FRandRange(
        1.0f - Variation,
        1.0f + Variation);
}
