#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCoreTypes.h"
#include "NocturneStrobeLight.generated.h"

class USceneComponent;
class USpotLightComponent;
class UAudioComponent;
class UCanvas;
class USoundBase;
class AHUD;

UENUM(BlueprintType)
enum class ENocturneStrobePattern : uint8
{
    Regular UMETA(DisplayName = "Regular"),
    DoubleFlash UMETA(DisplayName = "Double Flash"),
    Irregular UMETA(DisplayName = "Irregular")
};

UENUM(BlueprintType)
enum class ENocturneStrobeColorMode : uint8
{
    Fixed UMETA(DisplayName = "Fixed"),
    CosmicViolet UMETA(DisplayName = "Cosmic Violet")
};

UENUM(BlueprintType)
enum class ENocturneStrobeMotionMode : uint8
{
    Oscillate UMETA(DisplayName = "Oscillate"),
    ContinuousRotation UMETA(DisplayName = "Axial Spin")
};

UCLASS(Blueprintable)
class NOCTURNESTROBE_API ANocturneStrobeLight : public AActor
{
    GENERATED_BODY()

public:
    ANocturneStrobeLight();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void PostRenderFor(
        APlayerController* PlayerController,
        UCanvas* Canvas,
        FVector CameraPosition,
        FVector CameraDirection) override;

    UFUNCTION(BlueprintCallable, Category = "Nocturne Strobe|Control")
    void SetStrobeEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Nocturne Strobe|Control")
    void ToggleStrobe();

    UFUNCTION(BlueprintCallable, Category = "Nocturne Strobe|Control")
    void TriggerFlash(float Duration = 0.08f);

    UFUNCTION(BlueprintCallable, Category = "Nocturne Strobe|Control")
    void RefreshSettings();

    UFUNCTION(BlueprintCallable, Category = "Nocturne Strobe|Control")
    void ToggleControlsPanel();

    UFUNCTION(BlueprintPure, Category = "Nocturne Strobe|Control")
    bool IsStrobeEnabled() const { return bRuntimeEnabled; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nocturne Strobe|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nocturne Strobe|Components")
    TObjectPtr<USceneComponent> SweepPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nocturne Strobe|Components")
    TObjectPtr<USpotLightComponent> StrobeLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nocturne Strobe|Components")
    TObjectPtr<UAudioComponent> DirectionalAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Control")
    bool bStartEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Control")
    bool bEnableKeyboardToggle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Control", meta = (EditCondition = "bEnableKeyboardToggle"))
    FKey ToggleKey = EKeys::L;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Controls Panel")
    bool bShowControlsPanelAtStart = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Controls Panel")
    bool bEnableControlsPanelToggle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Controls Panel", meta = (EditCondition = "bEnableControlsPanelToggle"))
    FKey ControlsPanelKey = EKeys::K;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio")
    TObjectPtr<USoundBase> SoundSource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio")
    bool bAutoPlayAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio")
    bool bLoopAudio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio")
    bool bAudioFollowsStrobeState = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio", meta = (ClampMin = "0.0", ClampMax = "4.0"))
    float AudioVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio", meta = (ClampMin = "0.1", ClampMax = "4.0"))
    float AudioPitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio|Direction", meta = (ClampMin = "0.0", Units = "cm"))
    float AudioInnerRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio|Direction", meta = (ClampMin = "0.0", Units = "cm"))
    float AudioFalloffDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio|Direction", meta = (ClampMin = "0.0", ClampMax = "180.0", Units = "deg"))
    float AudioInnerConeAngle = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio|Direction", meta = (ClampMin = "0.0", ClampMax = "180.0", Units = "deg"))
    float AudioOuterConeAngle = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio|Direction", meta = (ClampMin = "0.0", Units = "cm"))
    float AudioConeOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio|Direction", meta = (ClampMin = "0.0", Units = "cm"))
    float AudioRearBleedRadius = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio|Direction", meta = (ClampMin = "0.0", Units = "cm"))
    float AudioRearBleedFalloffDistance = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Audio|Direction")
    bool bEnableAudioOcclusion = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light", meta = (ClampMin = "0.0", Units = "lm"))
    float MaximumIntensity = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light", meta = (ClampMin = "0.0", Units = "lm"))
    float MinimumIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light")
    FLinearColor LightColor = FLinearColor(0.62f, 0.72f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Color")
    ENocturneStrobeColorMode ColorMode = ENocturneStrobeColorMode::Fixed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Color", meta = (EditCondition = "ColorMode == ENocturneStrobeColorMode::CosmicViolet", EditConditionHides))
    FLinearColor CosmicVioletColor = FLinearColor(0.17f, 0.005f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Color", meta = (EditCondition = "ColorMode == ENocturneStrobeColorMode::CosmicViolet", EditConditionHides))
    FLinearColor CosmicMagentaColor = FLinearColor(1.0f, 0.005f, 0.38f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Color", meta = (EditCondition = "ColorMode == ENocturneStrobeColorMode::CosmicViolet", EditConditionHides))
    FLinearColor CosmicPurpleColor = FLinearColor(0.52f, 0.015f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Color", meta = (EditCondition = "ColorMode == ENocturneStrobeColorMode::CosmicViolet", EditConditionHides, ClampMin = "0.01", ClampMax = "10.0", Units = "Hz"))
    float CosmicColorCycleSpeed = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Color", meta = (EditCondition = "ColorMode == ENocturneStrobeColorMode::CosmicViolet", EditConditionHides, ClampMin = "0.0", ClampMax = "1.0"))
    float CosmicColorIrregularity = 0.38f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light")
    bool bUseTemperature = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light", meta = (ClampMin = "1000.0", ClampMax = "15000.0", Units = "K"))
    float Temperature = 9000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light", meta = (ClampMin = "0.0", Units = "cm"))
    float AttenuationRadius = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light", meta = (ClampMin = "0.0", ClampMax = "80.0", Units = "deg"))
    float InnerConeAngle = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light", meta = (ClampMin = "1.0", ClampMax = "89.0", Units = "deg"))
    float OuterConeAngle = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light", meta = (ClampMin = "0.0", Units = "cm"))
    float SourceRadius = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float VolumetricScatteringIntensity = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light")
    bool bCastShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Light")
    bool bCastVolumetricShadow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Pattern")
    ENocturneStrobePattern Pattern = ENocturneStrobePattern::DoubleFlash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Pattern", meta = (ClampMin = "0.1", ClampMax = "30.0", Units = "Hz"))
    float Frequency = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Pattern", meta = (ClampMin = "0.01", ClampMax = "0.95"))
    float DutyCycle = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Pattern", meta = (ClampMin = "0.0", ClampMax = "0.5", Units = "s"))
    float DoubleFlashGap = 0.065f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Pattern", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Irregularity = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Pattern", meta = (ClampMin = "1.0", ClampMax = "200.0"))
    float IntensityResponseSpeed = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Pattern")
    int32 RandomSeed = 1337;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Pattern", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InitialPhase = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Sweep")
    bool bEnableSweep = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Sweep", meta = (EditCondition = "bEnableSweep"))
    ENocturneStrobeMotionMode MotionMode = ENocturneStrobeMotionMode::Oscillate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Sweep")
    FRotator BaseLightRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Sweep", meta = (ClampMin = "0.0", ClampMax = "90.0", Units = "deg"))
    float YawAmplitude = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Sweep", meta = (ClampMin = "0.0", ClampMax = "45.0", Units = "deg"))
    float PitchAmplitude = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Sweep", meta = (ClampMin = "0.0", ClampMax = "5.0", Units = "Hz"))
    float SweepFrequency = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Sweep", meta = (ClampMin = "0.0", ClampMax = "360.0", Units = "deg"))
    float SweepPhaseDegrees = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Sweep|Continuous Rotation", meta = (EditCondition = "bEnableSweep && MotionMode == ENocturneStrobeMotionMode::ContinuousRotation", Units = "deg/s"))
    float ContinuousPitchSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Sweep|Continuous Rotation", meta = (EditCondition = "bEnableSweep && MotionMode == ENocturneStrobeMotionMode::ContinuousRotation", Units = "deg/s"))
    float ContinuousYawSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Strobe|Sweep|Continuous Rotation", meta = (EditCondition = "bEnableSweep && MotionMode == ENocturneStrobeMotionMode::ContinuousRotation", Units = "deg/s"))
    float ContinuousRollSpeed = 60.0f;

private:
    bool bRuntimeEnabled = true;
    float RuntimeTime = 0.0f;
    float CurrentIntensity = 0.0f;
    float ManualFlashRemaining = 0.0f;
    float IrregularPulseRemaining = 0.0f;
    float TimeUntilIrregularPulse = 0.0f;
    FRandomStream RandomStream;
    bool bControlsPanelVisible = true;
    TWeakObjectPtr<AHUD> RegisteredHUD;

    bool EvaluateRegularPulse() const;
    bool EvaluateDoubleFlashPulse() const;
    bool EvaluateIrregularPulse(float DeltaSeconds);
    void UpdateLight(float DeltaSeconds);
    void UpdateLightColor();
    void UpdateSweep();
    void RefreshAudioSettings();
    void UpdateAudioPlaybackState();
    void EnsureHUDRegistration();
    void ScheduleNextIrregularPulse();
};
