using UnrealBuildTool;

public class EclipseRotF : ModuleRules
{
    public EclipseRotF(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore",
            "EnhancedInput", "Niagara", "NiagaraCore",
            "GameplayTags", "GameplayTasks", "GameplayAbilities",
            "DeveloperSettings", "MoviePlayer",
        });
        
        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate", "SlateCore", "UMG", "AIModule",
        });

        // Slice code under Source/EclipseRotF/Aura/Slices/... includes its headers by their
        // module-relative path (Aura/Slices/<Slice>/<Header>.h).
        PublicIncludePaths.Add(ModuleDirectory);
    }
}
