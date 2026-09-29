#include "NocturnePlayerGameMode.h"

#include "NocturnePlayerCharacter.h"
#include "NocturnePlayerHUD.h"
#include "UObject/ConstructorHelpers.h"

ANocturnePlayerGameMode::ANocturnePlayerGameMode()
{
    static ConstructorHelpers::FClassFinder<APawn> EditablePlayerClass(
        TEXT("/Game/NocturneLook/Player/BP_NocturnePlayerCharacter"));

    DefaultPawnClass = ANocturnePlayerCharacter::StaticClass();
    if (EditablePlayerClass.Succeeded())
    {
        DefaultPawnClass = EditablePlayerClass.Class;
    }

    HUDClass = ANocturnePlayerHUD::StaticClass();
}
