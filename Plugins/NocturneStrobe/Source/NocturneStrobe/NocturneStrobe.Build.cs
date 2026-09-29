using UnrealBuildTool;

public class NocturneStrobe : ModuleRules
{
    public NocturneStrobe(ReadOnlyTargetRules Target) : base(Target)
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
