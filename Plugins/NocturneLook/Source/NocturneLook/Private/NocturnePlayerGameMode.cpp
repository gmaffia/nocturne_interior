#include "NocturnePlayerGameMode.h"

#include "NocturnePlayerCharacter.h"
#include "NocturnePlayerHUD.h"

ANocturnePlayerGameMode::ANocturnePlayerGameMode()
{
    DefaultPawnClass = ANocturnePlayerCharacter::StaticClass();
    HUDClass = ANocturnePlayerHUD::StaticClass();
}
