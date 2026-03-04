using UnrealBuildTool;

public class SeagullStorm : ModuleRules
{
    public SeagullStorm(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "Paper2D",
            "UMG",
            "Slate",
            "SlateCore",
            "HorizonSDK"
        });
    }
}
