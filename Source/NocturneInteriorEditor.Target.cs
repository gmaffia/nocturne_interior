using UnrealBuildTool;
using System.Collections.Generic;

public class NocturneInteriorEditorTarget : TargetRules
{
    public NocturneInteriorEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("NocturneInterior");
    }
}
