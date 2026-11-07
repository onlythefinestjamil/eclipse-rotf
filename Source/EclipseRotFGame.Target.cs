using UnrealBuildTool;

public class EclipseRotFGameTarget : TargetRules
{
    public EclipseRotFGameTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        ExtraModuleNames.Add("EclipseRotF");
    }
}
