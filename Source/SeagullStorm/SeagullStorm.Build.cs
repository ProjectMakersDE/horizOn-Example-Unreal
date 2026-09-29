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

        // horizOn Validated Actions (server-checked runs for the leaderboard). Needs a horizOn SDK
        // with UHorizonValidatedActionsManager (the first SDK release after 1.6.0 that ships
        // TASK-883). The vendored plugin in Plugins/HorizonSDK does not have it yet, so the switch
        // stays off and the game uses the normal SubmitScore path. Set it to true once the plugin
        // is updated. See README.md, section "Validated Actions".
        const bool bWithValidatedActions = false;
        PublicDefinitions.Add("HORIZON_WITH_VALIDATED_ACTIONS=" + (bWithValidatedActions ? "1" : "0"));
    }
}
