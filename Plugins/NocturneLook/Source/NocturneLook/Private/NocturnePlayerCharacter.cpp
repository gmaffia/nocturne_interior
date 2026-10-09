#include "NocturnePlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/LightComponent.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "InputCoreTypes.h"
#include "Misc/ConfigCacheIni.h"

ANocturnePlayerCharacter::ANocturnePlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(34.0f, 72.0f);

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = WalkSpeed;
    Movement->MaxAcceleration = MovementAcceleration;
    Movement->BrakingDecelerationWalking = BrakingDeceleration;
    Movement->GroundFriction = GroundFriction;
    Movement->bUseSeparateBrakingFriction = true;
    Movement->BrakingFriction = 7.0f;
    Movement->GravityScale = GravityScale;
    Movement->MaxStepHeight = 30.0f;
    Movement->bUseFlatBaseForFloorChecks = true;
    Movement->JumpZVelocity = 0.0f;

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(CameraForwardOffset, 0.0f, CameraHeight));
    FirstPersonCamera->bUsePawnControlRotation = true;
}

void ANocturnePlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitializeGraphicsQuality();
    InitializeGraphicsFeatures();

    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        FInputModeGameOnly InputMode;
        InputMode.SetConsumeCaptureMouseDown(true);
        PlayerController->SetInputMode(InputMode);
        PlayerController->SetShowMouseCursor(false);
    }

    if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
    {
        ViewportClient->SetMouseCaptureMode(
            EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
        ViewportClient->SetMouseLockMode(EMouseLockMode::LockAlways);

        if (FViewport* Viewport = ViewportClient->Viewport)
        {
            Viewport->CaptureMouse(true);
            Viewport->LockMouseToViewport(true);
        }
    }

    CurrentStance = InitialStance;
    FirstPersonCamera->SetRelativeLocation(FVector(
        CameraForwardOffset,
        0.0f,
        GetStanceCameraHeight()));
}

void ANocturnePlayerCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    MouseCaptureRefreshAccumulator += DeltaSeconds;
    if (MouseCaptureRefreshAccumulator >= 0.25f)
    {
        MouseCaptureRefreshAccumulator = 0.0f;

        if (UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport())
        {
            ViewportClient->SetMouseCaptureMode(
                EMouseCaptureMode::CapturePermanently_IncludingInitialMouseDown);
            ViewportClient->SetMouseLockMode(EMouseLockMode::LockAlways);

            if (FViewport* Viewport = ViewportClient->Viewport;
                Viewport && Viewport->HasFocus() && Viewport->IsForegroundWindow())
            {
                if (!Viewport->HasMouseCapture())
                {
                    Viewport->CaptureMouse(true);
                }
                Viewport->LockMouseToViewport(true);
            }
        }
    }

    if (bRecenterMouseWhileCaptured)
    {
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
        FViewport* Viewport = ViewportClient ? ViewportClient->Viewport : nullptr;
        if (PlayerController && Viewport &&
            Viewport->HasFocus() && Viewport->IsForegroundWindow())
        {
            const FIntPoint ViewportSize = Viewport->GetSizeXY();
            if (ViewportSize.X > 0 && ViewportSize.Y > 0)
            {
                // Synergy can transfer the cursor when it reaches a physical
                // display edge even while Unreal requests viewport locking.
                PlayerController->SetMouseLocation(
                    ViewportSize.X / 2,
                    ViewportSize.Y / 2);
            }
        }
    }

    GraphicsFeatureRefreshAccumulator += DeltaSeconds;
    if (GraphicsFeatureRefreshAccumulator >= 1.0f)
    {
        GraphicsFeatureRefreshAccumulator = 0.0f;
        ApplyActorGraphicsFeatures();
    }

    UCharacterMovementComponent* Movement = GetCharacterMovement();
    Movement->MaxWalkSpeed = GetStanceMovementSpeed();
    Movement->MaxAcceleration = MovementAcceleration;
    Movement->BrakingDecelerationWalking = BrakingDeceleration;
    Movement->GroundFriction = GroundFriction;
    Movement->GravityScale = GravityScale;

    const FVector HorizontalVelocity(GetVelocity().X, GetVelocity().Y, 0.0f);
    const float Speed = HorizontalVelocity.Size();
    const bool bIsWalking = Movement->IsMovingOnGround() && Speed > 8.0f;

    FVector TargetLocation(CameraForwardOffset, 0.0f, GetStanceCameraHeight());
    if (bIsWalking)
    {
        const float SpeedRatio = FMath::Clamp(
            Speed / FMath::Max(GetStanceMovementSpeed(), 1.0f),
            0.0f,
            1.0f);
        WalkCycle += DeltaSeconds * FMath::Lerp(6.5f, 10.0f, SpeedRatio);
        TargetLocation.Y += FMath::Sin(WalkCycle * 0.5f) * HeadBobSideAmount;
        TargetLocation.Z += FMath::Abs(FMath::Sin(WalkCycle)) * HeadBobAmount - (HeadBobAmount * 0.6875f);
    }

    FirstPersonCamera->SetRelativeLocation(FMath::VInterpTo(
        FirstPersonCamera->GetRelativeLocation(),
        TargetLocation,
        DeltaSeconds,
        bIsWalking ? 11.0f : StanceTransitionSpeed));
}

void ANocturnePlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ANocturnePlayerCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ANocturnePlayerCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAction(
        TEXT("RaiseStance"),
        IE_Pressed,
        this,
        &ANocturnePlayerCharacter::RaiseStance);
    PlayerInputComponent->BindAction(
        TEXT("LowerStance"),
        IE_Pressed,
        this,
        &ANocturnePlayerCharacter::LowerStance);
    PlayerInputComponent->BindAction(
        TEXT("ToggleControlHints"),
        IE_Pressed,
        this,
        &ANocturnePlayerCharacter::ToggleControlHints);
    PlayerInputComponent->BindKey(
        EKeys::G,
        IE_Pressed,
        this,
        &ANocturnePlayerCharacter::CycleGraphicsQuality);
    PlayerInputComponent->BindKey(EKeys::P, IE_Pressed, this, &ANocturnePlayerCharacter::ToggleGraphicsFeaturePanel);
    PlayerInputComponent->BindKey(EKeys::J, IE_Pressed, this, &ANocturnePlayerCharacter::ToggleLumenHardwareRayTracing);
    PlayerInputComponent->BindKey(EKeys::V, IE_Pressed, this, &ANocturnePlayerCharacter::ToggleVolumetricFog);
    PlayerInputComponent->BindKey(EKeys::T, IE_Pressed, this, &ANocturnePlayerCharacter::ToggleTelevisionShadows);
    PlayerInputComponent->BindKey(EKeys::Y, IE_Pressed, this, &ANocturnePlayerCharacter::ToggleStrobeShadows);
    PlayerInputComponent->BindKey(EKeys::B, IE_Pressed, this, &ANocturnePlayerCharacter::ToggleStrobeVolumetricBeam);
}

void ANocturnePlayerCharacter::MoveForward(float Value)
{
    if (!FMath::IsNearlyZero(Value))
    {
        const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::X), Value);
    }
}

void ANocturnePlayerCharacter::MoveRight(float Value)
{
    if (!FMath::IsNearlyZero(Value))
    {
        const FRotator MovementRotation(0.0f, GetControlRotation().Yaw, 0.0f);
        AddMovementInput(FRotationMatrix(MovementRotation).GetUnitAxis(EAxis::Y), Value);
    }
}

void ANocturnePlayerCharacter::RaiseStance()
{
    switch (CurrentStance)
    {
    case ENocturnePlayerStance::Crouched:
        CurrentStance = ENocturnePlayerStance::Seated;
        break;
    case ENocturnePlayerStance::Seated:
        CurrentStance = ENocturnePlayerStance::Standing;
        break;
    case ENocturnePlayerStance::Standing:
        break;
    }
}

void ANocturnePlayerCharacter::LowerStance()
{
    switch (CurrentStance)
    {
    case ENocturnePlayerStance::Standing:
        CurrentStance = ENocturnePlayerStance::Seated;
        break;
    case ENocturnePlayerStance::Seated:
        CurrentStance = ENocturnePlayerStance::Crouched;
        break;
    case ENocturnePlayerStance::Crouched:
        break;
    }
}

void ANocturnePlayerCharacter::ToggleControlHints()
{
    bShowControlHints = !bShowControlHints;
}

void ANocturnePlayerCharacter::InitializeGraphicsQuality()
{
    UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
    if (!Settings)
    {
        CurrentGraphicsQualityIndex = 2;
        return;
    }

    Settings->LoadSettings(false);

    float CurrentScaleNormalized = 1.0f;
    float CurrentScaleValue = 100.0f;
    float MinimumScaleValue = 50.0f;
    float MaximumScaleValue = 100.0f;
    Settings->GetResolutionScaleInformationEx(
        CurrentScaleNormalized,
        CurrentScaleValue,
        MinimumScaleValue,
        MaximumScaleValue);

    if (CurrentScaleValue >= 87.5f)
    {
        CurrentGraphicsQualityIndex = 2;
    }
    else if (CurrentScaleValue >= 62.5f)
    {
        CurrentGraphicsQualityIndex = 1;
    }
    else
    {
        CurrentGraphicsQualityIndex = 0;
    }

    ApplyGraphicsQuality();
}

void ANocturnePlayerCharacter::CycleGraphicsQuality()
{
    CurrentGraphicsQualityIndex = (CurrentGraphicsQualityIndex + 1) % 3;
    ApplyGraphicsQuality();
}

void ANocturnePlayerCharacter::ApplyGraphicsQuality()
{
    UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
    if (!Settings)
    {
        return;
    }

    CurrentGraphicsQualityIndex = FMath::Clamp(CurrentGraphicsQualityIndex, 0, 2);

    static constexpr float ResolutionScales[] = {50.0f, 75.0f, 100.0f};
    static constexpr int32 VolumeGridPixelSizes[] = {8, 6, 4};
    static constexpr int32 VolumeGridDepths[] = {128, 160, 192};
    static constexpr float LocalShadowLodBiases[] = {1.0f, 0.0f, -1.0f};
    static constexpr int32 LocalShadowRayCounts[] = {8, 10, 12};
    static constexpr int32 LocalShadowSamplesPerRay[] = {4, 6, 8};
    static constexpr int32 VirtualShadowPageBudgets[] = {4096, 6144, 8192};

    Settings->SetResolutionScaleValueEx(ResolutionScales[CurrentGraphicsQualityIndex]);
    Settings->ApplyNonResolutionSettings();

    const auto SetIntegerConsoleVariable = [](const TCHAR* Name, int32 Value)
    {
        if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
        {
            Variable->Set(Value, ECVF_SetByGameSetting);
        }
    };
    const auto SetFloatConsoleVariable = [](const TCHAR* Name, float Value)
    {
        if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
        {
            Variable->Set(Value, ECVF_SetByGameSetting);
        }
    };

    SetIntegerConsoleVariable(
        TEXT("r.VolumetricFog.GridPixelSize"),
        VolumeGridPixelSizes[CurrentGraphicsQualityIndex]);
    SetIntegerConsoleVariable(
        TEXT("r.VolumetricFog.GridSizeZ"),
        VolumeGridDepths[CurrentGraphicsQualityIndex]);
    SetFloatConsoleVariable(
        TEXT("r.Shadow.Virtual.ResolutionLodBiasLocal"),
        LocalShadowLodBiases[CurrentGraphicsQualityIndex]);
    SetFloatConsoleVariable(
        TEXT("r.Shadow.Virtual.ResolutionLodBiasLocalMoving"),
        LocalShadowLodBiases[CurrentGraphicsQualityIndex]);
    SetIntegerConsoleVariable(
        TEXT("r.Shadow.Virtual.SMRT.RayCountLocal"),
        LocalShadowRayCounts[CurrentGraphicsQualityIndex]);
    SetIntegerConsoleVariable(
        TEXT("r.Shadow.Virtual.SMRT.SamplesPerRayLocal"),
        LocalShadowSamplesPerRay[CurrentGraphicsQualityIndex]);
    SetIntegerConsoleVariable(
        TEXT("r.Shadow.Virtual.MaxPhysicalPages"),
        VirtualShadowPageBudgets[CurrentGraphicsQualityIndex]);

    Settings->SaveSettings();
}

void ANocturnePlayerCharacter::InitializeGraphicsFeatures()
{
    const auto GetConsoleVariableState = [](const TCHAR* Name, bool DefaultValue)
    {
        if (const IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
        {
            return Variable->GetInt() != 0;
        }
        return DefaultValue;
    };

    bLumenHardwareRayTracingEnabled = GetConsoleVariableState(TEXT("r.Lumen.HardwareRayTracing"), true);
    bVolumetricFogEnabled = GetConsoleVariableState(TEXT("r.VolumetricFog"), true);

    if (GConfig)
    {
        constexpr const TCHAR* Section = TEXT("NocturneGraphicsFeatures");
        GConfig->GetBool(Section, TEXT("ShowPanel"), bShowGraphicsFeaturePanel, GGameUserSettingsIni);
        GConfig->GetBool(Section, TEXT("LumenHardwareRayTracing"), bLumenHardwareRayTracingEnabled, GGameUserSettingsIni);
        GConfig->GetBool(Section, TEXT("VolumetricFog"), bVolumetricFogEnabled, GGameUserSettingsIni);
        GConfig->GetBool(Section, TEXT("TelevisionShadows"), bTelevisionShadowsEnabled, GGameUserSettingsIni);
        GConfig->GetBool(Section, TEXT("StrobeShadows"), bStrobeShadowsEnabled, GGameUserSettingsIni);
        GConfig->GetBool(Section, TEXT("StrobeVolumetricBeam"), bStrobeVolumetricBeamEnabled, GGameUserSettingsIni);
    }

    ApplyGraphicsFeatures();
}

void ANocturnePlayerCharacter::SaveGraphicsFeatures() const
{
    if (!GConfig)
    {
        return;
    }

    constexpr const TCHAR* Section = TEXT("NocturneGraphicsFeatures");
    GConfig->SetBool(Section, TEXT("ShowPanel"), bShowGraphicsFeaturePanel, GGameUserSettingsIni);
    GConfig->SetBool(Section, TEXT("LumenHardwareRayTracing"), bLumenHardwareRayTracingEnabled, GGameUserSettingsIni);
    GConfig->SetBool(Section, TEXT("VolumetricFog"), bVolumetricFogEnabled, GGameUserSettingsIni);
    GConfig->SetBool(Section, TEXT("TelevisionShadows"), bTelevisionShadowsEnabled, GGameUserSettingsIni);
    GConfig->SetBool(Section, TEXT("StrobeShadows"), bStrobeShadowsEnabled, GGameUserSettingsIni);
    GConfig->SetBool(Section, TEXT("StrobeVolumetricBeam"), bStrobeVolumetricBeamEnabled, GGameUserSettingsIni);
    GConfig->Flush(false, GGameUserSettingsIni);
}

void ANocturnePlayerCharacter::ApplyGraphicsFeatures()
{
    const auto SetConsoleVariable = [](const TCHAR* Name, bool bEnabled)
    {
        if (IConsoleVariable* Variable = IConsoleManager::Get().FindConsoleVariable(Name))
        {
            Variable->Set(bEnabled ? 1 : 0, ECVF_SetByGameSetting);
        }
    };

    SetConsoleVariable(TEXT("r.Lumen.HardwareRayTracing"), bLumenHardwareRayTracingEnabled);
    SetConsoleVariable(TEXT("r.VolumetricFog"), bVolumetricFogEnabled);
    ApplyActorGraphicsFeatures();
}

void ANocturnePlayerCharacter::ApplyActorGraphicsFeatures()
{
    if (!GetWorld())
    {
        return;
    }

    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
        {
            continue;
        }

        const FString ClassName = Actor->GetClass()->GetName();
        const bool bIsTelevision = ClassName.Contains(TEXT("NocturneTelevisionScreen"));
        const bool bIsStrobe = ClassName.Contains(TEXT("NocturneStrobeLight"));
        if (!bIsTelevision && !bIsStrobe)
        {
            continue;
        }

        TInlineComponentArray<ULightComponent*> LightComponents(Actor);
        for (ULightComponent* LightComponent : LightComponents)
        {
            if (!LightComponent)
            {
                continue;
            }

            if (bIsTelevision)
            {
                LightComponent->SetCastShadows(bTelevisionShadowsEnabled);
            }
            else
            {
                LightComponent->SetCastShadows(bStrobeShadowsEnabled);
                const float CurrentScatteringIntensity = LightComponent->VolumetricScatteringIntensity;
                if (!bCapturedStrobeVolumetricScatteringIntensity &&
                    CurrentScatteringIntensity > 0.0f)
                {
                    StrobeVolumetricScatteringIntensity = CurrentScatteringIntensity;
                    bCapturedStrobeVolumetricScatteringIntensity = true;
                }

                if (bStrobeVolumetricBeamEnabled)
                {
                    LightComponent->SetVolumetricScatteringIntensity(StrobeVolumetricScatteringIntensity);
                }
                else
                {
                    if (CurrentScatteringIntensity > 0.0f)
                    {
                        StrobeVolumetricScatteringIntensity = CurrentScatteringIntensity;
                        bCapturedStrobeVolumetricScatteringIntensity = true;
                    }
                    LightComponent->SetVolumetricScatteringIntensity(0.0f);
                }
            }
        }
    }
}

void ANocturnePlayerCharacter::ToggleGraphicsFeaturePanel()
{
    bShowGraphicsFeaturePanel = !bShowGraphicsFeaturePanel;
    SaveGraphicsFeatures();
}

void ANocturnePlayerCharacter::ToggleLumenHardwareRayTracing()
{
    bLumenHardwareRayTracingEnabled = !bLumenHardwareRayTracingEnabled;
    ApplyGraphicsFeatures();
    SaveGraphicsFeatures();
}

void ANocturnePlayerCharacter::ToggleVolumetricFog()
{
    bVolumetricFogEnabled = !bVolumetricFogEnabled;
    ApplyGraphicsFeatures();
    SaveGraphicsFeatures();
}

void ANocturnePlayerCharacter::ToggleTelevisionShadows()
{
    bTelevisionShadowsEnabled = !bTelevisionShadowsEnabled;
    ApplyActorGraphicsFeatures();
    SaveGraphicsFeatures();
}

void ANocturnePlayerCharacter::ToggleStrobeShadows()
{
    bStrobeShadowsEnabled = !bStrobeShadowsEnabled;
    ApplyActorGraphicsFeatures();
    SaveGraphicsFeatures();
}

void ANocturnePlayerCharacter::ToggleStrobeVolumetricBeam()
{
    bStrobeVolumetricBeamEnabled = !bStrobeVolumetricBeamEnabled;
    ApplyActorGraphicsFeatures();
    SaveGraphicsFeatures();
}

float ANocturnePlayerCharacter::GetStanceCameraHeight() const
{
    switch (CurrentStance)
    {
    case ENocturnePlayerStance::Standing:
        return StandingCameraHeight;
    case ENocturnePlayerStance::Crouched:
        return CrouchedCameraHeight;
    case ENocturnePlayerStance::Seated:
    default:
        return SeatedCameraHeight;
    }
}

float ANocturnePlayerCharacter::GetStanceMovementSpeed() const
{
    switch (CurrentStance)
    {
    case ENocturnePlayerStance::Seated:
        return 0.0f;
    case ENocturnePlayerStance::Crouched:
        return WalkSpeed * CrouchedMovementSpeedMultiplier;
    case ENocturnePlayerStance::Standing:
    default:
        return WalkSpeed;
    }
}

FText ANocturnePlayerCharacter::GetCurrentStanceDisplayName() const
{
    switch (CurrentStance)
    {
    case ENocturnePlayerStance::Standing:
        return FText::FromString(TEXT("Standing"));
    case ENocturnePlayerStance::Crouched:
        return FText::FromString(TEXT("Crouched"));
    case ENocturnePlayerStance::Seated:
    default:
        return FText::FromString(TEXT("Seated"));
    }
}

FText ANocturnePlayerCharacter::GetGraphicsQualityDisplayName() const
{
    switch (CurrentGraphicsQualityIndex)
    {
    case 0:
        return FText::FromString(TEXT("Performance 50% / Low lighting"));
    case 1:
        return FText::FromString(TEXT("High 75% / High lighting"));
    case 2:
    default:
        return FText::FromString(TEXT("Native 100% / Ultra lighting"));
    }
}
