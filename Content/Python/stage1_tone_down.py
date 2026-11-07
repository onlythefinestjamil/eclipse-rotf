"""Eclipse Stage 1 — Tone down lighting. Remove the point light, reduce intensities."""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_tone_down.log")

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
log("ECLIPSE STAGE 1 — Tone Down Lighting")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

actors = ELL.get_all_level_actors()
log(f"Total actors: {len(actors)}")

# 1. Delete the overpowered point light
log("1. Removing point light fill...")
for actor in actors:
    try:
        if actor.get_class().get_name() == "PointLight":
            label = actor.get_actor_label()
            if "Fill" in label or "fill" in label:
                actor.destroy_actor()
                log(f"  + Deleted {label}")
    except:
        pass

# 2. Reduce all directional lights to reasonable values
log("2. Reducing directional lights...")
for actor in ELL.get_all_level_actors():
    try:
        cls = actor.get_class().get_name()
        if cls == "DirectionalLight":
            comp = actor.get_editor_property("light_component")
            if comp:
                comp.set_editor_property("intensity", 8.0)  # Was 50, too bright
                comp.set_editor_property("cast_shadows", True)
                actor.set_actor_rotation(Rot(-55, 45, 0))
                log(f"  + {actor.get_actor_label()} reduced to intensity=8")
        elif cls == "SkyLight":
            comp = actor.get_editor_property("light_component")
            if comp:
                comp.set_editor_property("intensity", 1.5)  # Was 10, too bright
                log(f"  + {actor.get_actor_label()} reduced to intensity=1.5")
    except:
        pass

# 3. Reduce fog back to atmospheric but not blinding
log("3. Adjusting fog...")
for actor in ELL.get_all_level_actors():
    try:
        if actor.get_class().get_name() == "ExponentialHeightFog":
            comp = actor.get_editor_property("component")
            if comp:
                comp.set_editor_property("fog_density", 0.01)
                comp.set_editor_property("fog_max_opacity", 0.4)
                comp.set_editor_property("start_distance", 800.0)
                log("  + Fog adjusted (density=0.01, opacity=0.4)")
    except:
        pass

# 4. Take screenshot
log("4. Taking screenshot...")
try:
    unreal.AutomationLibrary.take_high_res_screenshot(
        1920, 1080,
        "C:/Users/Tap/Desktop/Stage1_TonedDown.png"
    )
    log("  + Screenshot saved!")
except Exception as e:
    log(f"  ! Screenshot failed: {e}", "ERROR")

log("=" * 60)
log("TONE DOWN COMPLETE")
log("=" * 60)
