"""
Eclipse Stage 1 — Replace basic shapes with AAA models.
Remove cube/cylinder cathedral, position AAA Aetherborn + Berserker properly.
Run inside UE5.8:  py C:/Users/Tap/EclipseRotF/Content/Python/stage1_aaa_replace.py
"""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_aaa_replace.log")

def log(msg, level="INFO"):
    line = f"[{datetime.now().strftime('%H:%M:%S.%f')[:-3]}] [{level}] {msg}"
    print(line)
    with open(LOG, "a", encoding="utf-8") as f:
        f.write(line + "\n")

try:
    import unreal
except ImportError:
    log("ERROR: unreal module not available!")
    raise

log("=" * 60)
log("ECLIPSE STAGE 1 — Replace Basic Shapes with AAA Models")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

actors = ELL.get_all_level_actors()
log(f"Current actors in level: {len(actors)}")

# 1. Remove all basic cube/cylinder cathedral pieces
log("1. Removing basic shape cathedral pieces...")
removed = 0
for actor in actors:
    try:
        label = actor.get_actor_label()
        cls = actor.get_class().get_name()
        if cls == "StaticMeshActor" and ("Solara_Floor" in label or "Solara_Ceiling" in label or
            "Solara_Wall" in label or "Solara_Pillar" in label):
            actor.destroy_actor()
            removed += 1
            log(f"  - Removed {label}")
    except:
        pass
log(f"  Total removed: {removed}")

# 2. Find and reposition the Aetherborn player
log("2. Positioning Aetherborn player...")
for actor in ELL.get_all_level_actors():
    try:
        label = actor.get_actor_label()
        if "Aetherborn" in label or "Stage1_Player" in label:
            actor.set_actor_location(Vec(0, 0, 90))
            actor.set_actor_rotation(Rot(0, 0, 0))
            actor.set_actor_scale3d(Vec(1, 1, 1))
            log(f"  + {label} positioned at origin")
            # Make sure it's visible
            comp = actor.get_editor_property("static_mesh_component") if hasattr(actor, "get_editor_property") else None
            if comp:
                try:
                    comp.set_editor_property("visibility", True)
                except:
                    pass
    except:
        pass

# 3. Find and reposition the Berserker enemy
log("3. Positioning Berserker enemy...")
for actor in ELL.get_all_level_actors():
    try:
        label = actor.get_actor_label()
        if "Berserker" in label or "Stage1_Enemy" in label or "BERSERKER" in label:
            actor.set_actor_location(Vec(1500, 1500, 90))
            actor.set_actor_rotation(Rot(0, 180, 0))
            actor.set_actor_scale3d(Vec(1, 1, 1))
            log(f"  + {label} positioned at (1500, 1500)")
            comp = actor.get_editor_property("static_mesh_component") if hasattr(actor, "get_editor_property") else None
            if comp:
                try:
                    comp.set_editor_property("visibility", True)
                except:
                    pass
    except:
        pass

# 4. Check all remaining actors
log("4. Remaining actors:")
remaining = ELL.get_all_level_actors()
log(f"  Total: {len(remaining)}")
for a in remaining:
    try:
        log(f"  - {a.get_actor_label()} ({a.get_class().get_name()})")
    except:
        pass

# 5. Take screenshot
log("5. Taking screenshot...")
try:
    unreal.AutomationLibrary.take_high_res_screenshot(
        1920, 1080,
        "C:/Users/Tap/Desktop/Stage1_AAA_Replace.png"
    )
    log("  + Screenshot saved!")
except Exception as e:
    log(f"  ! Screenshot failed: {e}", "ERROR")

log("=" * 60)
log("AAA REPLACE COMPLETE")
log("=" * 60)
