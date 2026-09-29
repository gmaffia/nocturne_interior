#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NocturnePlayerHUD.generated.h"

UCLASS()
class NOCTURNELOOK_API ANocturnePlayerHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;
};
