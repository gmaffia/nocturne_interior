using UnrealBuildTool;

public class NocturneInterior : ModuleRules
{
    public NocturneInterior(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore"
        });
    }
}
