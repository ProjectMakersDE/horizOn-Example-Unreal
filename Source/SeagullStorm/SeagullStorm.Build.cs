using UnrealBuildTool;

public class SeagullStorm : ModuleRules
{
    public SeagullStorm(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateIncludePaths.Add(ModuleDirectory);

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
            "HorizonSDK",
            "Json",
            "JsonUtilities"
        });

        // Official SDK embeds tickets, server-owned state and evidence upload.
        PublicDefinitions.Add("HORIZON_WITH_VALIDATED_ACTIONS=1");
    }
}
