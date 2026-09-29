#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NocturneLookController.generated.h"

class UPostProcessComponent;
class USceneComponent;

UCLASS(Blueprintable)
class NOCTURNELOOK_API ANocturneLookController : public AActor
{
    GENERATED_BODY()

public:
    ANocturneLookController();

    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Look|General")
    bool bLookEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Look|General")
    bool bEnableRuntimeControls = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Look|General")
    bool bShowRuntimePanel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Look|Image", meta = (ClampMin = "-3.0", ClampMax = "5.0", UIMin = "-3.0", UIMax = "2.0"))
    float ExposureBias = -1.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Look|Image", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FilmGrainIntensity = 0.65f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Look|Image", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VignetteIntensity = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Look|Image", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float ChromaticAberrationIntensity = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Look|Color", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Saturation = 0.72f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Look|Color", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Contrast = 1.06f;

    UFUNCTION(BlueprintCallable, Category = "Nocturne Look")
    void AdjustExposure(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Nocturne Look")
    void AdjustFilmGrain(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Nocturne Look")
    void AdjustVignette(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Nocturne Look")
    void AdjustChromaticAberration(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Nocturne Look")
    void ToggleLookEnabled();

    UFUNCTION(BlueprintCallable, Category = "Nocturne Look")
    void ResetLook();

    UFUNCTION(BlueprintPure, Category = "Nocturne Look")
    float GetExposure() const;

    UFUNCTION(BlueprintPure, Category = "Nocturne Look")
    float GetFilmGrain() const;

    UFUNCTION(BlueprintPure, Category = "Nocturne Look")
    float GetVignette() const;

    UFUNCTION(BlueprintPure, Category = "Nocturne Look")
    float GetChromaticAberration() const;

    UFUNCTION(BlueprintPure, Category = "Nocturne Look")
    bool IsLookEnabled() const { return bLookEnabled; }

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Nocturne Look")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Nocturne Look")
    UPostProcessComponent* PostProcess;

    FPostProcessSettings LookSettings;

    void BuildSettings();
    void ApplyActiveSettings();
    void HandleRuntimeControls();
    void DrawRuntimePanel() const;
};
