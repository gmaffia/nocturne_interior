using UnrealBuildTool;
using System.Collections.Generic;

public class NocturneInteriorTarget : TargetRules
{
    public NocturneInteriorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        bOverrideBuildEnvironment = true;
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            GlobalDefinitions.Add("__has_feature(x)=0");
        }
        ExtraModuleNames.Add("NocturneInterior");
    }
}
