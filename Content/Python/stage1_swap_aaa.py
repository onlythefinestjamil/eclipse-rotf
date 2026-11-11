"""Swap the AAA Aetherborn into the level — it's already imported."""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_swap_aaa.log")
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
log("ECLIPSE — Swap AAA Aetherborn into Level")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

# The AAA mesh IS imported — load it
AAA_PATH = "/Game/Characters/AETHERBORN_HERO_ALT_AAA/StaticMeshes/AETHERBORN_HERO_AAA.AETHERBORN_HERO_AAA"
log(f"Loading AAA mesh: {AAA_PATH}")
aaa_mesh = EAL.load_asset(AAA_PATH)

if aaa_mesh:
    log(f"  + AAA mesh loaded: {aaa_mesh.get_name()}")
    log(f"  + Type: {aaa_mesh.get_class().get_name()}")
    
    # Remove old Aetherborn actors
    for actor in ELL.get_all_level_actors():
        try:
            label = actor.get_actor_label()
            if "Aetherborn" in label or "Stage1_Player" in label:
                actor.destroy_actor()
                log(f"  - Removed: {label}")
        except:
            pass
    
    # Spawn AAA Aetherborn
    actor = ELL.spawn_actor_from_class(unreal.StaticMeshActor, Vec(0, 0, 0), Rot(0, 0, 0))
    if actor:
        actor.set_actor_label("Aetherborn_Hero_AAA")
        comp = actor.get_editor_property("static_mesh_component")
        if comp:
            comp.set_static_mesh(aaa_mesh)
            try:
                comp.set_editor_property("cast_shadow", True)
            except:
                pass
        log(f"  + AAA Aetherborn spawned at origin!")
    
    # List actors
    log("All actors:")
    for a in ELL.get_all_level_actors():
        try:
            loc = a.get_actor_location()
            log(f"  - {a.get_actor_label()} ({a.get_class().get_name()}) @ ({int(loc.x)}, {int(loc.y)}, {int(loc.z)})")
        except:
            pass
    
    # Screenshot
    log("Taking screenshot...")
    try:
        unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "C:/Users/Tap/Desktop/Stage1_Aetherborn_AAA.png")
        log("  + Screenshot saved!")
    except Exception as e:
        log(f"  ! Screenshot failed: {e}", "ERROR")
else:
    log("  ! AAA mesh not found!", "ERROR")

log("=" * 60)
log("SWAP COMPLETE")
log("=" * 60)