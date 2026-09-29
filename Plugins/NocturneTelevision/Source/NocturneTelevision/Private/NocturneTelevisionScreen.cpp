#include "NocturneTelevisionScreen.h"

#include "Components/AudioComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaSource.h"
#include "MediaTexture.h"
#include "RHIGPUReadback.h"
#include "RenderingThread.h"
#include "UObject/ConstructorHelpers.h"

#include <atomic>

struct FNocturneReactiveReadbackState
{
    FNocturneReactiveReadbackState()
        : TextureReadback(MakeShared<FRHIGPUTextureReadback, ESPMode::ThreadSafe>(
              TEXT("NocturneTelevisionReactiveLight")))
        , Future(Promise.GetFuture())
    {
    }

    ~FNocturneReactiveReadbackState()
    {
        if (!bValueSet.load())
        {
            Promise.SetValue(FLinearColor::Black);
        }
    }

    TSharedRef<FRHIGPUTextureReadback, ESPMode::ThreadSafe> TextureReadback;
    TPromise<FLinearColor> Promise;
    TFuture<FLinearColor> Future;
    std::atomic<bool> bPollInFlight{false};
    std::atomic<bool> bValueSet{false};
};

namespace NocturneTelevisionParameters
{
    static const FName VideoTexture(TEXT("VideoTexture"));
    static const FName EmissiveIntensity(TEXT("EmissiveIntensity"));
    static const FName DistortionEnabled(TEXT("DistortionEnabled"));
    static const FName DistortionAmount(TEXT("DistortionAmount"));
    static const FName ScanlineIntensity(TEXT("ScanlineIntensity"));
    static const FName ScanlineDensity(TEXT("ScanlineDensity"));
    static const FName NoiseIntensity(TEXT("NoiseIntensity"));
    static const FName HorizontalJitter(TEXT("HorizontalJitter"));
    static const FName VerticalRollSpeed(TEXT("VerticalRollSpeed"));
    static const FName ChromaticAberration(TEXT("ChromaticAberration"));
    static const FName FlickerIntensity(TEXT("FlickerIntensity"));
    static const FName VignetteIntensity(TEXT("VignetteIntensity"));
    static const FName TrackingBarIntensity(TEXT("TrackingBarIntensity"));
}

ANocturneTelevisionScreen::ANocturneTelevisionScreen()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    ScreenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScreenMesh"));
    ScreenMesh->SetupAttachment(SceneRoot);
    ScreenMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
    ScreenMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ScreenMesh->SetCastShadow(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(
        TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (PlaneMesh.Succeeded())
    {
        ScreenMesh->SetStaticMesh(PlaneMesh.Object);
    }

    ScreenLight = CreateDefaultSubobject<URectLightComponent>(TEXT("ScreenLight"));
    ScreenLight->SetupAttachment(SceneRoot);
    ScreenLight->SetRelativeLocation(FVector(0.0f, 0.0f, 2.0f));
    ScreenLight->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));

    MediaSound = CreateDefaultSubobject<UMediaSoundComponent>(TEXT("MediaSound"));
    MediaSound->SetupAttachment(SceneRoot);
    NormalizeAudioComponentRuntimeFlags();
}

void ANocturneTelevisionScreen::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    NormalizeAudioComponentRuntimeFlags();
    UpdateScreenGeometry();
    UpdateScreenLight();
    UpdateScreenAppearance();
    UpdateAudio();
    InitializeReactiveLight();
}

void ANocturneTelevisionScreen::BeginPlay()
{
    Super::BeginPlay();

    EnsureRuntimeComponents();
    UpdateScreenGeometry();
    UpdateScreenLight();
    NormalizeAudioComponentRuntimeFlags();
    InitializeRuntimeMedia();
    if (IsValid(MediaPlayer))
    {
        MediaPlayer->SetLooping(bLoop);
    }
    UpdateScreenAppearance();
    InitializeReactiveLight();
    UpdateAudio();

    if (bAutoPlay)
    {
        OpenVideoSource();
    }
}

void ANocturneTelevisionScreen::EnsureRuntimeComponents()
{
    if (!IsValid(SceneRoot))
    {
        SceneRoot = Cast<USceneComponent>(GetRootComponent());
    }

    if (!IsValid(SceneRoot))
    {
        SceneRoot = NewObject<USceneComponent>(this, TEXT("RuntimeSceneRoot"));
        AddInstanceComponent(SceneRoot);
        SceneRoot->RegisterComponent();
        SetRootComponent(SceneRoot);
    }

    if (!IsValid(ScreenMesh))
    {
        TInlineComponentArray<UStaticMeshComponent*> MeshComponents(this);
        for (UStaticMeshComponent* MeshComponent : MeshComponents)
        {
            if (IsValid(MeshComponent) && MeshComponent->GetFName() == TEXT("ScreenMesh"))
            {
                ScreenMesh = MeshComponent;
                break;
            }
        }
    }

    if (!IsValid(ScreenMesh))
    {
        ScreenMesh = NewObject<UStaticMeshComponent>(this, TEXT("RuntimeScreenMesh"));
        ScreenMesh->SetupAttachment(SceneRoot);
        ScreenMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));
        ScreenMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        ScreenMesh->SetCastShadow(false);
        ScreenMesh->SetStaticMesh(LoadObject<UStaticMesh>(
            nullptr,
            TEXT("/Engine/BasicShapes/Plane.Plane")));
        AddInstanceComponent(ScreenMesh);
        ScreenMesh->RegisterComponent();
    }

    if (!IsValid(ScreenLight))
    {
        TInlineComponentArray<URectLightComponent*> LightComponents(this);
        for (URectLightComponent* LightComponent : LightComponents)
        {
            if (IsValid(LightComponent) &&
                (LightComponent->GetFName() == TEXT("ScreenLight") ||
                 LightComponent->GetFName() == TEXT("RuntimeScreenLight")))
            {
                ScreenLight = LightComponent;
                break;
            }
        }
    }

    if (!IsValid(ScreenLight))
    {
        ScreenLight = NewObject<URectLightComponent>(this, TEXT("RuntimeScreenLight"));
        ScreenLight->SetupAttachment(SceneRoot);
        ScreenLight->SetRelativeLocation(FVector(0.0f, 0.0f, 2.0f));
        ScreenLight->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
        ScreenLight->SetMobility(EComponentMobility::Movable);
        ScreenLight->SetIntensityUnits(ELightUnits::Unitless);
        ScreenLight->SetAffectGlobalIllumination(true);
        ScreenLight->SetAffectReflection(true);
        AddInstanceComponent(ScreenLight);
        ScreenLight->RegisterComponent();
    }

    if (!IsValid(MediaSound))
    {
        MediaSound = NewObject<UMediaSoundComponent>(this, TEXT("RuntimeMediaSound"));
        MediaSound->SetupAttachment(SceneRoot);
        AddInstanceComponent(MediaSound);
        MediaSound->RegisterComponent();
    }
}

void ANocturneTelevisionScreen::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (MediaSound)
    {
        MediaSound->SetMediaPlayer(nullptr);
    }
    if (MediaTexture)
    {
        MediaTexture->SetMediaPlayer(nullptr);
    }
    if (MediaPlayer)
    {
        MediaPlayer->Close();
    }

    MediaTexture = nullptr;
    MediaPlayer = nullptr;
    DynamicScreenMaterial = nullptr;
    bMediaTextureBound = false;
    LightSampleRenderTarget = nullptr;
    ReactiveReadbackState.Reset();

    Super::EndPlay(EndPlayReason);
}

void ANocturneTelevisionScreen::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bMediaTextureBound &&
        IsValid(DynamicScreenMaterial) &&
        IsValid(MediaPlayer) &&
        MediaPlayer->IsPlaying() &&
        IsValid(MediaTexture) &&
        MediaTexture->GetResource() &&
        MediaTexture->GetWidth() > 1 &&
        MediaTexture->GetHeight() > 1)
    {
        DynamicScreenMaterial->SetTextureParameterValue(
            NocturneTelevisionParameters::VideoTexture,
            MediaTexture);
        bMediaTextureBound = true;

        UE_LOG(
            LogTemp,
            Display,
            TEXT("Nocturne Television: bound decoded video frame %dx%d to material %s."),
            MediaTexture->GetWidth(),
            MediaTexture->GetHeight(),
            *GetNameSafe(DynamicScreenMaterial));
    }

    UpdateReactiveLight(DeltaSeconds);
}

void ANocturneTelevisionScreen::NormalizeAudioComponentRuntimeFlags()
{
    if (!MediaSound)
    {
        return;
    }

    MediaSound->bIsEditorOnly = false;
#if WITH_EDITORONLY_DATA
    MediaSound->SetIsVisualizationComponent(false);
#endif

    if (UAudioComponent* AudioComponent = MediaSound->GetAudioComponent())
    {
        AudioComponent->bIsEditorOnly = false;
#if WITH_EDITORONLY_DATA
        AudioComponent->SetIsVisualizationComponent(false);
#endif
    }
}

bool ANocturneTelevisionScreen::Play()
{
    InitializeRuntimeMedia();
    if (!MediaPlayer->IsReady())
    {
        if (!VideoSource)
        {
            return false;
        }
        MediaPlayer->PlayOnOpen = true;
        MediaPlayer->SetLooping(bLoop);
        return MediaPlayer->OpenSource(VideoSource);
    }
    return MediaPlayer->Play();
}

bool ANocturneTelevisionScreen::Pause()
{
    return MediaPlayer && MediaPlayer->Pause();
}

void ANocturneTelevisionScreen::Stop()
{
    if (MediaPlayer)
    {
        MediaPlayer->Close();
    }
}

void ANocturneTelevisionScreen::TogglePlayback()
{
    if (MediaPlayer && MediaPlayer->IsPlaying())
    {
        Pause();
    }
    else
    {
        Play();
    }
}

bool ANocturneTelevisionScreen::OpenVideoSource()
{
    if (!VideoSource)
    {
        return false;
    }

    InitializeRuntimeMedia();
    MediaPlayer->PlayOnOpen = bAutoPlay;
    MediaPlayer->SetLooping(bLoop);
    return MediaPlayer->OpenSource(VideoSource);
}

void ANocturneTelevisionScreen::RefreshVisualSettings()
{
    UpdateScreenGeometry();
    UpdateScreenLight();
    UpdateScreenAppearance();
}

void ANocturneTelevisionScreen::InitializeRuntimeMedia()
{
    if (!IsValid(MediaPlayer))
    {
        MediaPlayer = NewObject<UMediaPlayer>(this, NAME_None, RF_Transient);
    }

    if (!IsValid(MediaTexture))
    {
        MediaTexture = NewObject<UMediaTexture>(this, NAME_None, RF_Transient);
        if (IsValid(MediaTexture))
        {
            MediaTexture->AutoClear = true;
            MediaTexture->ClearColor = FLinearColor::Black;
            // The distortion material samples VideoTexture as a regular Texture2D.
            MediaTexture->NewStyleOutput = true;
            MediaTexture->SetMediaPlayer(MediaPlayer);
            MediaTexture->UpdateResource();
        }
    }

    bMediaTextureBound = false;

    if (IsValid(MediaSound) && IsValid(MediaPlayer))
    {
        MediaSound->SetMediaPlayer(MediaPlayer);
    }
}

void ANocturneTelevisionScreen::InitializeReactiveLight()
{
    if (!bEnableReactiveLight || LightSampleRenderTarget)
    {
        return;
    }

    LightSampleRenderTarget = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);
    LightSampleRenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
    LightSampleRenderTarget->ClearColor = FLinearColor::Black;
    LightSampleRenderTarget->InitAutoFormat(8, 8);
    LightSampleRenderTarget->UpdateResourceImmediate(true);

    SmoothedReactiveIntensity = LightIntensity;
    TargetReactiveIntensity = LightIntensity;
    SmoothedReactiveColor = LightColor;
    TargetReactiveColor = LightColor;
}

void ANocturneTelevisionScreen::SampleReactiveLight()
{
    if (!LightSampleRenderTarget || !DynamicScreenMaterial || ReactiveReadbackState.IsValid())
    {
        return;
    }

    UKismetRenderingLibrary::DrawMaterialToRenderTarget(
        this,
        LightSampleRenderTarget,
        DynamicScreenMaterial);

    FTextureRenderTargetResource* RenderTargetResource =
        LightSampleRenderTarget->GameThread_GetRenderTargetResource();
    if (!RenderTargetResource)
    {
        return;
    }

    ReactiveReadbackState =
        MakeShared<FNocturneReactiveReadbackState, ESPMode::ThreadSafe>();
    const TSharedRef<FNocturneReactiveReadbackState, ESPMode::ThreadSafe> ReadbackState =
        ReactiveReadbackState.ToSharedRef();

    ENQUEUE_RENDER_COMMAND(NocturneTelevisionEnqueueReactiveReadback)(
        [RenderTargetResource, ReadbackState](FRHICommandListImmediate& RHICmdList)
        {
            FRHITexture* RenderTargetTexture = RenderTargetResource->GetRenderTargetTexture();
            RHICmdList.Transition(FRHITransitionInfo(
                RenderTargetTexture,
                ERHIAccess::SRVMask,
                ERHIAccess::CopySrc));
            ReadbackState->TextureReadback->EnqueueCopy(
                RHICmdList,
                RenderTargetTexture,
                FResolveRect(0, 0, 8, 8));
            RHICmdList.Transition(FRHITransitionInfo(
                RenderTargetTexture,
                ERHIAccess::CopySrc,
                ERHIAccess::SRVMask));
        });
}

void ANocturneTelevisionScreen::ConsumeReactiveLightSample()
{
    if (!ReactiveReadbackState.IsValid())
    {
        return;
    }

    if (ReactiveReadbackState->Future.IsReady())
    {
        const FLinearColor AverageColor = ReactiveReadbackState->Future.Get();
        ReactiveReadbackState.Reset();

        const float SafeEmissiveIntensity = FMath::Max(EmissiveIntensity, 0.01f);
        FLinearColor NormalizedColor = AverageColor;
        NormalizedColor.R /= SafeEmissiveIntensity;
        NormalizedColor.G /= SafeEmissiveIntensity;
        NormalizedColor.B /= SafeEmissiveIntensity;
        NormalizedColor.A = 1.0f;

        const float SampledBrightness = FMath::Clamp(
            NormalizedColor.GetLuminance() * ReactiveLightBrightnessGain,
            ReactiveLightMinimumBrightness,
            ReactiveLightMaximumBrightness);
        const float BrightnessMultiplier = FMath::Lerp(
            1.0f,
            SampledBrightness,
            FMath::Clamp(ReactiveLightIntensityStrength, 0.0f, 1.0f));

        const FLinearColor ClampedSampleColor(
            FMath::Clamp(NormalizedColor.R, 0.0f, 1.0f),
            FMath::Clamp(NormalizedColor.G, 0.0f, 1.0f),
            FMath::Clamp(NormalizedColor.B, 0.0f, 1.0f),
            1.0f);

        TargetReactiveIntensity = LightIntensity * BrightnessMultiplier;
        TargetReactiveColor = FMath::Lerp(
            LightColor,
            ClampedSampleColor,
            FMath::Clamp(ReactiveLightColorStrength, 0.0f, 1.0f));

        if (!bLoggedFirstReactiveSample)
        {
            bLoggedFirstReactiveSample = true;
            UE_LOG(
                LogTemp,
                Display,
                TEXT("Nocturne Television: reactive light sample intensity=%.2f color=(%.3f, %.3f, %.3f)."),
                TargetReactiveIntensity,
                TargetReactiveColor.R,
                TargetReactiveColor.G,
                TargetReactiveColor.B);
        }
        return;
    }

    bool bExpected = false;
    if (!ReactiveReadbackState->bPollInFlight.compare_exchange_strong(bExpected, true))
    {
        return;
    }

    const TSharedRef<FNocturneReactiveReadbackState, ESPMode::ThreadSafe> ReadbackState =
        ReactiveReadbackState.ToSharedRef();
    ENQUEUE_RENDER_COMMAND(NocturneTelevisionPollReactiveReadback)(
        [ReadbackState](FRHICommandListImmediate& RHICmdList)
        {
            if (!ReadbackState->TextureReadback->IsReady())
            {
                ReadbackState->bPollInFlight.store(false);
                return;
            }

            int32 RowPitchInPixels = 0;
            int32 BufferHeight = 0;
            void* ReadbackBuffer = ReadbackState->TextureReadback->Lock(
                RowPitchInPixels,
                &BufferHeight);

            FLinearColor AverageColor = FLinearColor::Black;
            if (ReadbackBuffer && RowPitchInPixels >= 8 && BufferHeight >= 8)
            {
                const FColor* Pixels = static_cast<const FColor*>(ReadbackBuffer);
                for (int32 Y = 0; Y < 8; ++Y)
                {
                    for (int32 X = 0; X < 8; ++X)
                    {
                        AverageColor += FLinearColor::FromSRGBColor(
                            Pixels[Y * RowPitchInPixels + X]);
                    }
                }
                AverageColor /= 64.0f;
            }

            ReadbackState->TextureReadback->Unlock();
            ReadbackState->bValueSet.store(true);
            ReadbackState->Promise.SetValue(AverageColor);
        });
}

void ANocturneTelevisionScreen::UpdateReactiveLight(float DeltaSeconds)
{
    if (!bEnableScreenLight || !bEnableReactiveLight ||
        !IsValid(ScreenLight) || !IsValid(DynamicScreenMaterial))
    {
        return;
    }

    InitializeReactiveLight();
    ConsumeReactiveLightSample();
    ReactiveLightSampleAccumulator += DeltaSeconds;
    const float SampleInterval = 1.0f / FMath::Max(ReactiveLightSampleRate, 1.0f);
    if (ReactiveLightSampleAccumulator >= SampleInterval)
    {
        ReactiveLightSampleAccumulator = FMath::Fmod(ReactiveLightSampleAccumulator, SampleInterval);
        SampleReactiveLight();
    }

    const float SmoothingAlpha = 1.0f - FMath::Exp(
        -FMath::Max(ReactiveLightSmoothingSpeed, 0.1f) * DeltaSeconds);
    SmoothedReactiveIntensity = FMath::Lerp(
        SmoothedReactiveIntensity,
        TargetReactiveIntensity,
        SmoothingAlpha);
    SmoothedReactiveColor = FMath::Lerp(
        SmoothedReactiveColor,
        TargetReactiveColor,
        SmoothingAlpha);

    ScreenLight->SetIntensity(SmoothedReactiveIntensity);
    ScreenLight->SetLightColor(SmoothedReactiveColor);
}

void ANocturneTelevisionScreen::UpdateScreenAppearance()
{
    if (!IsValid(ScreenMesh))
    {
        DynamicScreenMaterial = nullptr;
        bMediaTextureBound = false;
        return;
    }

    UMaterialInterface* MaterialToUse = ScreenMaterial;
    if (!MaterialToUse)
    {
        MaterialToUse = LoadObject<UMaterialInterface>(
            nullptr,
            TEXT("/NocturneTelevision/Materials/M_NocturneTelevisionDistorted.M_NocturneTelevisionDistorted"));
    }

    if (!IsValid(MaterialToUse))
    {
        UE_LOG(LogTemp, Error, TEXT("Nocturne Television: screen material could not be loaded."));
        return;
    }

    const UWorld* World = GetWorld();
    if (!World || !World->IsGameWorld() || !IsValid(MediaTexture))
    {
        DynamicScreenMaterial = nullptr;
        bMediaTextureBound = false;
        ScreenMesh->SetMaterial(0, MaterialToUse);
        return;
    }

    DynamicScreenMaterial = UMaterialInstanceDynamic::Create(MaterialToUse, this);
    bMediaTextureBound = false;
    if (IsValid(DynamicScreenMaterial))
    {
        ScreenMesh->SetMaterial(0, DynamicScreenMaterial);

        // Bind only after the media player has delivered a decoded frame. The
        // initial resource is merely the clear texture and is replaced later.
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::EmissiveIntensity,
            EmissiveIntensity);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::DistortionEnabled,
            bEnableDistortion ? 1.0f : 0.0f);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::DistortionAmount,
            DistortionAmount);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::ScanlineIntensity,
            ScanlineIntensity);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::ScanlineDensity,
            ScanlineDensity);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::NoiseIntensity,
            NoiseIntensity);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::HorizontalJitter,
            HorizontalJitter);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::VerticalRollSpeed,
            VerticalRollSpeed);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::ChromaticAberration,
            ChromaticAberration);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::FlickerIntensity,
            FlickerIntensity);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::VignetteIntensity,
            VignetteIntensity);
        DynamicScreenMaterial->SetScalarParameterValue(
            NocturneTelevisionParameters::TrackingBarIntensity,
            TrackingBarIntensity);
    }
}

void ANocturneTelevisionScreen::UpdateScreenGeometry()
{
    if (IsValid(ScreenMesh))
    {
        ScreenMesh->SetRelativeScale3D(FVector(ScreenWidth / 100.0f, ScreenHeight / 100.0f, 1.0f));
    }
}

void ANocturneTelevisionScreen::UpdateScreenLight()
{
    if (!IsValid(ScreenLight))
    {
        return;
    }

    ScreenLight->SetVisibility(bEnableScreenLight);
    ScreenLight->SetMobility(EComponentMobility::Movable);
    ScreenLight->SetIntensityUnits(ELightUnits::Unitless);
    ScreenLight->SetAffectGlobalIllumination(true);
    ScreenLight->SetAffectReflection(true);
    ScreenLight->SetIntensity(LightIntensity);
    ScreenLight->SetLightColor(LightColor);
    ScreenLight->SetAttenuationRadius(LightAttenuationRadius);
    ScreenLight->SetSourceWidth(ScreenWidth);
    ScreenLight->SetSourceHeight(ScreenHeight);
    ScreenLight->SetCastShadows(bCastScreenShadows);
}

void ANocturneTelevisionScreen::UpdateAudio()
{
    if (IsValid(MediaSound))
    {
        MediaSound->SetVolumeMultiplier(AudioVolume);
    }
}
