#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NocturnePlayerCharacter.generated.h"

class UCameraComponent;

UENUM(BlueprintType)
enum class ENocturnePlayerStance : uint8
{
    Crouched UMETA(DisplayName = "Crouched"),
    Seated UMETA(DisplayName = "Seated"),
    Standing UMETA(DisplayName = "Standing")
};

UCLASS(Blueprintable)
class NOCTURNELOOK_API ANocturnePlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ANocturnePlayerCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category = "Nocturne Player|Stance")
    ENocturnePlayerStance GetCurrentStance() const { return CurrentStance; }

    UFUNCTION(BlueprintPure, Category = "Nocturne Player|Stance")
    FText GetCurrentStanceDisplayName() const;

    UFUNCTION(BlueprintPure, Category = "Nocturne Player|Interface")
    bool ShouldShowControlHints() const { return bShowControlHints; }

    FText GetGraphicsQualityDisplayName() const;

    bool ShouldShowGraphicsFeaturePanel() const { return bShowGraphicsFeaturePanel; }
    bool IsLumenHardwareRayTracingEnabled() const { return bLumenHardwareRayTracingEnabled; }
    bool IsVolumetricFogEnabled() const { return bVolumetricFogEnabled; }
    bool AreTelevisionShadowsEnabled() const { return bTelevisionShadowsEnabled; }
    bool AreStrobeShadowsEnabled() const { return bStrobeShadowsEnabled; }
    bool IsStrobeVolumetricBeamEnabled() const { return bStrobeVolumetricBeamEnabled; }

protected:
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nocturne Player|Camera")
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Movement", meta = (ClampMin = "0.0"))
    float WalkSpeed = 155.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Movement", meta = (ClampMin = "0.0"))
    float MovementAcceleration = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Movement", meta = (ClampMin = "0.0"))
    float BrakingDeceleration = 2600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Movement", meta = (ClampMin = "0.0"))
    float GroundFriction = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Movement", meta = (ClampMin = "0.0"))
    float GravityScale = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Camera", meta = (ClampMin = "0.0"))
    float HeadBobAmount = 3.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Camera", meta = (ClampMin = "0.0"))
    float HeadBobSideAmount = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Camera", meta = (ClampMin = "-72.0", ClampMax = "120.0", Units = "cm"))
    float CameraHeight = 52.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Camera", meta = (ClampMin = "-50.0", ClampMax = "50.0", Units = "cm"))
    float CameraForwardOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Stance")
    ENocturnePlayerStance InitialStance = ENocturnePlayerStance::Seated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Stance", meta = (ClampMin = "-72.0", ClampMax = "120.0", Units = "cm"))
    float StandingCameraHeight = 88.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Stance", meta = (ClampMin = "-72.0", ClampMax = "120.0", Units = "cm"))
    float SeatedCameraHeight = 36.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Stance", meta = (ClampMin = "-72.0", ClampMax = "120.0", Units = "cm"))
    float CrouchedCameraHeight = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Stance", meta = (ClampMin = "0.1", ClampMax = "30.0"))
    float StanceTransitionSpeed = 7.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Stance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CrouchedMovementSpeedMultiplier = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nocturne Player|Interface")
    bool bShowControlHints = true;

private:
    float WalkCycle = 0.0f;
    float MouseCaptureRefreshAccumulator = 0.0f;
    float GraphicsFeatureRefreshAccumulator = 0.0f;
    bool bRecenterMouseWhileCaptured = true;
    bool bShowGraphicsFeaturePanel = true;
    bool bLumenHardwareRayTracingEnabled = true;
    bool bVolumetricFogEnabled = true;
    bool bTelevisionShadowsEnabled = true;
    bool bStrobeShadowsEnabled = true;
    bool bStrobeVolumetricBeamEnabled = true;
    bool bCapturedStrobeVolumetricScatteringIntensity = false;
    float StrobeVolumetricScatteringIntensity = 4.0f;
    int32 CurrentGraphicsQualityIndex = 2;
    ENocturnePlayerStance CurrentStance = ENocturnePlayerStance::Seated;

    void MoveForward(float Value);
    void MoveRight(float Value);
    void RaiseStance();
    void LowerStance();
    void ToggleControlHints();
    void InitializeGraphicsQuality();
    void CycleGraphicsQuality();
    void ApplyGraphicsQuality();
    void InitializeGraphicsFeatures();
    void SaveGraphicsFeatures() const;
    void ApplyGraphicsFeatures();
    void ApplyActorGraphicsFeatures();
    void ToggleGraphicsFeaturePanel();
    void ToggleLumenHardwareRayTracing();
    void ToggleVolumetricFog();
    void ToggleTelevisionShadows();
    void ToggleStrobeShadows();
    void ToggleStrobeVolumetricBeam();
    float GetStanceCameraHeight() const;
    float GetStanceMovementSpeed() const;
};
