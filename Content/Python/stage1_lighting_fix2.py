"""Eclipse Stage 1 — Fix lighting configuration on existing actors."""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_lighting_fix2.log")

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
log("ECLIPSE STAGE 1 — Lighting Fix v2")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
Vec = unreal.Vector
Rot = unreal.Rotator
LC = unreal.LinearColor

# Get all actors in the level
actors = ELL.get_all_level_actors()
log(f"Total actors in level: {len(actors)}")

# Find and configure existing lighting actors
sun_configured = False
sky_configured = False
fog_configured = False

for actor in actors:
    try:
        label = actor.get_actor_label()
        cls_name = actor.get_class().get_name()
        
        if cls_name == "DirectionalLight":
            log(f"  Found DirectionalLight: {label}")
            # Configure the sun
            actor.set_actor_rotation(Rot(-60, 45, 0))
            actor.set_actor_location(Vec(0, 0, 3000))
            try:
                comp = actor.get_editor_property("light_component")
                if comp:
                    comp.set_editor_property("intensity", 15.0)
                    comp.set_editor_property("light_color", LC(1.0, 0.92, 0.75, 1.0))
                    comp.set_editor_property("cast_shadows", True)
                    try:
                        comp.set_editor_property("shadow_resolution_scale", 4.0)
                    except:
                        pass
                    log("    + Sun configured (intensity=15, warm color, shadows on)")
            except Exception as e:
                log(f"    ! Sun config failed: {e}", "WARN")
            sun_configured = True
        
        elif cls_name == "SkyLight":
            log(f"  Found SkyLight: {label}")
            try:
                comp = actor.get_editor_property("light_component")
                if comp:
                    comp.set_editor_property("intensity", 3.0)
                    log("    + SkyLight configured (intensity=3.0)")
            except Exception as e:
                log(f"    ! SkyLight config failed: {e}", "WARN")
            sky_configured = True
        
        elif cls_name == "ExponentialHeightFog":
            log(f"  Found ExponentialHeightFog: {label}")
            try:
                comp = actor.get_editor_property("fog_component")
                if comp:
                    comp.set_editor_property("fog_density", 0.015)
                    comp.set_editor_property("fog_height_falloff", 0.4)
                    comp.set_editor_property("fog_max_opacity", 0.75)
                    comp.set_editor_property("start_distance", 300.0)
                    comp.set_editor_property("fog_inscattering_color", LC(0.5, 0.35, 0.25, 1.0))
                    try:
                        comp.set_editor_property("volumetric_fog_enable", True)
                        comp.set_editor_property("volumetric_fog_density", 0.06)
                        log("    + Volumetric fog enabled!")
                    except:
                        log("    ! Volumetric fog prop not available", "WARN")
                    log("    + Fog configured (density=0.015, volumetric on)")
            except Exception as e:
                log(f"    ! Fog config failed: {e}", "WARN")
            fog_configured = True
    except:
        pass

# If any lights are missing, spawn them
if not sun_configured:
    log("  Sun not found, spawning new one...")
    try:
        sun = ELL.spawn_actor_from_class(
            unreal.DirectionalLight,
            Vec(0, 0, 3000),
            Rot(-60, 45, 0),
        )
        if sun:
            sun.set_actor_label("Solara_Sun")
            comp = sun.get_editor_property("light_component")
            if comp:
                comp.set_editor_property("intensity", 15.0)
                comp.set_editor_property("light_color", LC(1.0, 0.92, 0.75, 1.0))
                comp.set_editor_property("cast_shadows", True)
            log("    + Sun spawned and configured!")
    except Exception as e:
        log(f"    ! Sun spawn failed: {e}", "ERROR")

if not sky_configured:
    log("  SkyLight not found, spawning new one...")
    try:
        sky = ELL.spawn_actor_from_class(
            unreal.SkyLight,
            Vec(0, 0, 4000),
            Rot(0, 0, 0),
        )
        if sky:
            sky.set_actor_label("Solara_SkyLight")
            comp = sky.get_editor_property("light_component")
            if comp:
                comp.set_editor_property("intensity", 3.0)
            log("    + SkyLight spawned and configured!")
    except Exception as e:
        log(f"    ! SkyLight spawn failed: {e}", "ERROR")

if not fog_configured:
    log("  Fog not found, spawning new one...")
    try:
        fog = ELL.spawn_actor_from_class(
            unreal.ExponentialHeightFog,
            Vec(0, 0, 0),
            Rot(0, 0, 0),
        )
        if fog:
            fog.set_actor_label("Solara_Fog")
            comp = fog.get_editor_property("fog_component")
            if comp:
                comp.set_editor_property("fog_density", 0.015)
                comp.set_editor_property("fog_height_falloff", 0.4)
                comp.set_editor_property("fog_max_opacity", 0.75)
                comp.set_editor_property("start_distance", 300.0)
                comp.set_editor_property("fog_inscattering_color", LC(0.5, 0.35, 0.25, 1.0))
                try:
                    comp.set_editor_property("volumetric_fog_enable", True)
                    comp.set_editor_property("volumetric_fog_density", 0.06)
                except:
                    pass
            log("    + Fog spawned and configured!")
    except Exception as e:
        log(f"    ! Fog spawn failed: {e}", "ERROR")

# Take a new screenshot with the fixed lighting
log("Taking screenshot with fixed lighting...")
try:
    unreal.AutomationLibrary.take_high_res_screenshot(
        1920, 1080,
        "C:/Users/Tap/Desktop/Stage1_Cathedral_LightingFixed.png"
    )
    log("  + Screenshot saved to Desktop!")
except Exception as e:
    log(f"  ! Screenshot failed: {e}", "WARN")

log("=" * 60)
log("LIGHTING FIX v2 COMPLETE")
log("=" * 60)
