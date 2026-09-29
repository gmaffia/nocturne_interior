using UnrealBuildTool;

public class NocturneTelevision : ModuleRules
{
    public NocturneTelevision(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "MediaAssets",
            "AudioMixer",
            "RHI",
            "RenderCore"
        });
    }
}
