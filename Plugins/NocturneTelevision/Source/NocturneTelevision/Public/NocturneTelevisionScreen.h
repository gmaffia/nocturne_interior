#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NocturneTelevisionScreen.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMediaPlayer;
class UMediaSoundComponent;
class UMediaSource;
class UMediaTexture;
class URectLightComponent;
class USceneComponent;
class UStaticMeshComponent;
class UTextureRenderTarget2D;
struct FNocturneReactiveReadbackState;

UCLASS(Blueprintable)
class NOCTURNETELEVISION_API ANocturneTelevisionScreen : public AActor
{
    GENERATED_BODY()

public:
    ANocturneTelevisionScreen();

    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintCallable, Category = "Nocturne Television|Playback")
    bool Play();

    UFUNCTION(BlueprintCallable, Category = "Nocturne Television|Playback")
    bool Pause();

    UFUNCTION(BlueprintCallable, Category = "Nocturne Television|Playback")
    void Stop();

    UFUNCTION(BlueprintCallable, Category = "Nocturne Television|Playback")
    void TogglePlayback();

    UFUNCTION(BlueprintCallable, Category = "Nocturne Television|Playback")
    bool OpenVideoSource();

    UFUNCTION(BlueprintCallable, Category = "Nocturne Television|Screen")
    void RefreshVisualSettings();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nocturne Television|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nocturne Television|Components")
    TObjectPtr<UStaticMeshComponent> ScreenMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nocturne Television|Components")
    TObjectPtr<URectLightComponent> ScreenLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nocturne Television|Components")
    TObjectPtr<UMediaSoundComponent> MediaSound;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Nocturne Television|Media")
    TObjectPtr<UMediaPlayer> MediaPlayer;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Nocturne Television|Media")
    TObjectPtr<UMediaTexture> MediaTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Media")
    TObjectPtr<UMediaSource> VideoSource;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Media")
    bool bAutoPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Media")
    bool bLoop = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Media", meta = (ClampMin = "0.0", ClampMax = "4.0"))
    float AudioVolume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Screen", meta = (ClampMin = "1.0", Units = "cm"))
    float ScreenWidth = 64.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Screen", meta = (ClampMin = "1.0", Units = "cm"))
    float ScreenHeight = 48.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Screen", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float EmissiveIntensity = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Screen")
    TObjectPtr<UMaterialInterface> ScreenMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion")
    bool bEnableDistortion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DistortionAmount = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScanlineIntensity = 0.22f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion", meta = (ClampMin = "20.0", ClampMax = "1000.0"))
    float ScanlineDensity = 280.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoiseIntensity = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion", meta = (ClampMin = "0.0", ClampMax = "0.1"))
    float HorizontalJitter = 0.012f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float VerticalRollSpeed = 0.015f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion", meta = (ClampMin = "0.0", ClampMax = "0.05"))
    float ChromaticAberration = 0.004f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FlickerIntensity = 0.06f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float VignetteIntensity = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Distortion", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TrackingBarIntensity = 0.18f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light")
    bool bEnableScreenLight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light", meta = (ClampMin = "0.0"))
    float LightIntensity = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light")
    FLinearColor LightColor = FLinearColor(0.34f, 0.46f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light", meta = (ClampMin = "0.0", Units = "cm"))
    float LightAttenuationRadius = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light")
    bool bCastScreenShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light|Reactive")
    bool bEnableReactiveLight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light|Reactive", meta = (ClampMin = "1.0", ClampMax = "30.0", Units = "Hz"))
    float ReactiveLightSampleRate = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light|Reactive", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ReactiveLightColorStrength = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light|Reactive", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ReactiveLightIntensityStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light|Reactive", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float ReactiveLightBrightnessGain = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light|Reactive", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float ReactiveLightMinimumBrightness = 0.12f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light|Reactive", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float ReactiveLightMaximumBrightness = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Television|Light|Reactive", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float ReactiveLightSmoothingSpeed = 8.0f;

private:
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> DynamicScreenMaterial;

    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> LightSampleRenderTarget;

    TSharedPtr<FNocturneReactiveReadbackState, ESPMode::ThreadSafe> ReactiveReadbackState;

    float ReactiveLightSampleAccumulator = 0.0f;
    bool bMediaTextureBound = false;
    bool bLoggedFirstReactiveSample = false;
    float SmoothedReactiveIntensity = 0.0f;
    float TargetReactiveIntensity = 0.0f;
    FLinearColor SmoothedReactiveColor = FLinearColor::Black;
    FLinearColor TargetReactiveColor = FLinearColor::Black;

    void EnsureRuntimeComponents();
    void InitializeRuntimeMedia();
    void InitializeReactiveLight();
    void SampleReactiveLight();
    void ConsumeReactiveLightSample();
    void UpdateReactiveLight(float DeltaSeconds);
    void UpdateScreenAppearance();
    void UpdateScreenGeometry();
    void UpdateScreenLight();
    void UpdateAudio();
    void NormalizeAudioComponentRuntimeFlags();
};
