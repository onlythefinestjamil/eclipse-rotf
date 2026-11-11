"""Take a high-quality screenshot of the Aetherborn AAA model in UE5.8."""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_aetherborn_screenshot.log")
if os.path.exists(LOG):
    os.remove(LOG)

def log(msg, level="INFO"):
    line = f"[{datetime.now().strftime('%H:%M:%S.%f')[:-3]}] [{level}] {msg}"
    print(line)
    with open(LOG, "a", encoding="utf-8") as f:
        f.write(line + "\n")

try:
    import unreal
except ImportError:
    log("ERROR: unreal not available!")
    raise

log("=" * 60)
log("Aetherborn AAA Screenshot")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

# Load the AAA Aetherborn mesh
AAA_PATH = "/Game/Characters/AETHERBORN_HERO_ALT_AAA/StaticMeshes/AETHERBORN_HERO_AAA.AETHERBORN_HERO_AAA"
log(f"Loading AAA mesh: {AAA_PATH}")
aaa_mesh = EAL.load_asset(AAA_PATH)

if not aaa_mesh:
    log("  ! AAA mesh not found, trying Aetherborn_male_HQ")
    AAA_PATH = "/Game/Characters/Aetherborn_male_HQ/StaticMeshes/Aetherborn_male_HQ.Aetherborn_male_HQ"
    aaa_mesh = EAL.load_asset(AAA_PATH)

if aaa_mesh:
    log(f"  + Mesh loaded: {aaa_mesh.get_name()}")
    
    # Remove all existing actors
    for actor in ELL.get_all_level_actors():
        try:
            actor.destroy_actor()
        except:
            pass
    log("  + Cleared all actors")
    
    # Spawn the Aetherborn
    actor = ELL.spawn_actor_from_class(unreal.StaticMeshActor, Vec(0, 0, 0), Rot(0, 0, 0))
    if actor:
        actor.set_actor_label("Aetherborn_AAA_Showcase")
        comp = actor.get_editor_property("static_mesh_component")
        if comp:
            comp.set_static_mesh(aaa_mesh)
            try:
                comp.set_editor_property("cast_shadow", True)
            except:
                pass
        log("  + Aetherborn spawned at origin")
    
    # Add clean 3-point lighting
    # Key light (front-right, warm)
    key = ELL.spawn_actor_from_class(unreal.DirectionalLight, Vec(500, -500, 1500), Rot(-45, 30, 0))
    if key:
        key.set_actor_label("Key_Light")
        comp = key.get_editor_property("light_component")
        if comp:
            comp.set_editor_property("intensity", 10.0)
        log("  + Key light: intensity=10, -45deg, warm")
    
    # Fill light (left, cool)
    fill = ELL.spawn_actor_from_class(unreal.DirectionalLight, Vec(-500, -300, 1000), Rot(-30, -30, 0))
    if fill:
        fill.set_actor_label("Fill_Light")
        comp = fill.get_editor_property("light_component")
        if comp:
            comp.set_editor_property("intensity", 3.0)
        log("  + Fill light: intensity=3, cool fill")
    
    # Rim light (behind, strong)
    rim = ELL.spawn_actor_from_class(unreal.DirectionalLight, Vec(0, 500, 2000), Rot(-60, 0, 0))
    if rim:
        rim.set_actor_label("Rim_Light")
        comp = rim.get_editor_property("light_component")
        if comp:
            comp.set_editor_property("intensity", 8.0)
        log("  + Rim light: intensity=8, behind")
    
    # Sky light for ambient
    sky = ELL.spawn_actor_from_class(unreal.SkyLight, Vec(0, 0, 3000), Rot(0, 0, 0))
    if sky:
        sky.set_actor_label("Sky_Light")
        comp = sky.get_editor_property("light_component")
        if comp:
            comp.set_editor_property("intensity", 2.0)
        log("  + Sky light: intensity=2")
    
    # Take screenshot
    log("Taking screenshot...")
    try:
        unreal.AutomationLibrary.take_high_res_screenshot(
            1920, 1080,
            "C:/Users/Tap/Desktop/Stage1_Aetherborn_AAA_Showcase.png"
        )
        log("  + Screenshot saved!")
    except Exception as e:
        log(f"  ! Screenshot failed: {e}", "ERROR")
    
    # List actors
    log("Actors in level:")
    for a in ELL.get_all_level_actors():
        try:
            log(f"  - {a.get_actor_label()} ({a.get_class().get_name()})")
        except:
            pass
else:
    log("  ! No mesh found!", "ERROR")

log("=" * 60)
log("SCREENSHOT COMPLETE")
log("=" * 60)
