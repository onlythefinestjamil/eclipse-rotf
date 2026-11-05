using UnrealBuildTool;

public class EclipseRotF : ModuleRules
{
    public EclipseRotF(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore",
            "EnhancedInput", "Niagara", "NiagaraCore",
            "GameplayTags", "GameplayTasks",
        });
        
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate", "SlateCore", "UMG",
        });
    }
}
