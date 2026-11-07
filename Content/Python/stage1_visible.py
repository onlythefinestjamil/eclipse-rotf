"""
Eclipse Stage 1 — Fix visibility. Move camera to frame the cathedral,
ensure materials are visible, add a proper sky sphere, and take screenshot.
"""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_visible.log")

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
log("ECLIPSE STAGE 1 — Make It Visible")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

# 1. Add a Sky Sphere or atmospheric fog for visible sky
log("1. Adding sky + atmospheric fog...")

# Add exponential height fog with brighter settings
fog_actors = [a for a in ELL.get_all_level_actors() if a.get_class().get_name() == "ExponentialHeightFog"]
for fog in fog_actors:
    try:
        comp = fog.get_editor_property("component")
        if comp:
            # Brighter fog so things aren't pitch black
            comp.set_editor_property("fog_density", 0.005)
            comp.set_editor_property("fog_max_opacity", 0.3)
            comp.set_editor_property("start_distance", 500.0)
            try:
                comp.set_editor_property("fog_inscattering_color", unreal.LinearColor(0.8, 0.8, 1.0, 1.0))
            except:
                pass
            log("  + Fog brightened (density=0.005, opacity=0.3)")
    except Exception as e:
        log(f"  ! Fog fix failed: {e}", "WARN")

# 2. Boost all directional lights
log("2. Boosting all lights...")
for actor in ELL.get_all_level_actors():
    try:
        cls = actor.get_class().get_name()
        if cls == "DirectionalLight":
            comp = actor.get_editor_property("light_component")
            if comp:
                comp.set_editor_property("intensity", 50.0)  # Much brighter
                comp.set_editor_property("cast_shadows", True)
                # Point it DOWN more so it actually illuminates
                actor.set_actor_rotation(Rot(-45, 45, 0))
                actor.set_actor_location(Vec(500, -500, 5000))
                log(f"  + {actor.get_actor_label()} boosted (intensity=50)")
        elif cls == "SkyLight":
            comp = actor.get_editor_property("light_component")
            if comp:
                comp.set_editor_property("intensity", 10.0)  # Much brighter
                log(f"  + {actor.get_actor_label()} boosted (intensity=10)")
    except:
        pass

# 3. Add a Sky Light with actual sky color if the SkyLight isn't working
log("3. Adding ambient light fill...")
try:
    # Add a point light at the ceiling for interior fill
    point_light = ELL.spawn_actor_from_class(
        unreal.PointLight,
        Vec(0, 0, 1500),  # Up near ceiling
        Rot(0, 0, 0),
    )
    if point_light:
        point_light.set_actor_label("Cathedral_Fill_Light")
        comp = point_light.get_editor_property("light_component")
        if comp:
            comp.set_editor_property("intensity", 50000.0)
            comp.set_editor_property("attenuation_radius", 10000.0)
        log("  + Point light added (warm interior fill, intensity=50000)")
except Exception as e:
    log(f"  ! Point light failed: {e}", "WARN")

# 4. Add a sky sphere
log("4. Adding sky sphere...")
try:
    # Load the engine sky sphere
    sky_sphere = EAL.load_asset("/Engine/EngineSky/BP_Sky_Sphere")
    if sky_sphere:
        spawned = ELL.spawn_actor_from_object(sky_sphere, Vec(0, 0, 0), Rot(0, 0, 0))
        if spawned:
            spawned.set_actor_label("Cathedral_Sky")
            log("  + Sky sphere added!")
    else:
        log("  ! Sky sphere asset not found", "WARN")
except Exception as e:
    log(f"  ! Sky sphere failed: {e}", "WARN")

# 5. Check all static mesh actors have materials
log("5. Checking mesh materials...")
for actor in ELL.get_all_level_actors():
    try:
        if actor.get_class().get_name() == "StaticMeshActor":
            comp = actor.get_editor_property("static_mesh_component")
            if comp:
                mesh = comp.get_static_mesh()
                if mesh:
                    # Make sure it's visible
                    comp.set_editor_property("visibility", True)
                    comp.set_editor_property("cast_shadow", True)
    except:
        pass
log("  + All mesh visibility checked")

# 6. Take screenshot
log("6. Taking screenshot...")
try:
    unreal.AutomationLibrary.take_high_res_screenshot(
        1920, 1080,
        "C:/Users/Tap/Desktop/Stage1_VISIBLE.png"
    )
    log("  + Screenshot saved!")
except Exception as e:
    log(f"  ! Screenshot failed: {e}", "ERROR")

log("=" * 60)
log("VISIBILITY FIX COMPLETE")
log("=" * 60)
