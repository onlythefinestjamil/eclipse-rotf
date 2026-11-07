using UnrealBuildTool;

public class EclipseRotFEditorTarget : TargetRules
{
    public EclipseRotFEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        bOverrideBuildEnvironment = true;
        ExtraModuleNames.Add("EclipseRotF");
    }
}
