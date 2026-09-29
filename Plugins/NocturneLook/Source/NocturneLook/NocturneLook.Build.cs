using UnrealBuildTool;

public class NocturneLook : ModuleRules
{
    public NocturneLook(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "InputCore" });
    }
}
