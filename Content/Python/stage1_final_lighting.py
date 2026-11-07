"""Eclipse Stage 1 — Final lighting fix with correct property names."""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_final_lighting.log")

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
log("ECLIPSE STAGE 1 — Final Lighting Fix")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

actors = ELL.get_all_level_actors()
log(f"Total actors: {len(actors)}")

# Find and fix all lighting actors
for actor in actors:
    try:
        cls_name = actor.get_class().get_name()
        label = actor.get_actor_label()
        
        if cls_name == "DirectionalLight":
            log(f"  Fixing DirectionalLight: {label}")
            # Get the light component
            comp = actor.get_editor_property("light_component")
            if comp:
                try:
                    comp.set_editor_property("intensity", 15.0)
                    log("    + intensity = 15.0")
                except Exception as e:
                    log(f"    ! intensity failed: {e}", "WARN")
                try:
                    comp.set_editor_property("cast_shadows", True)
                    log("    + cast_shadows = True")
                except Exception as e:
                    log(f"    ! cast_shadows failed: {e}", "WARN")
                # Try setting color via set_editor_property with a tuple
                try:
                    comp.set_editor_property("light_color", unreal.LinearColor(1.0, 0.92, 0.75, 1.0))
                    log("    + light_color set (warm)")
                except:
                    try:
                        # Try with FColor
                        comp.set_editor_property("light_color", (255, 235, 190, 255))
                        log("    + light_color set via tuple")
                    except:
                        log("    ! light_color failed (non-critical)", "WARN")
                # Set rotation for warm sun angle
                actor.set_actor_rotation(Rot(-60, 45, 0))
                log("    + rotation = (-60, 45, 0)")
        
        elif cls_name == "SkyLight":
            log(f"  Fixing SkyLight: {label}")
            comp = actor.get_editor_property("light_component")
            if comp:
                try:
                    comp.set_editor_property("intensity", 3.0)
                    log("    + intensity = 3.0")
                except Exception as e:
                    log(f"    ! intensity failed: {e}", "WARN")
        
        elif cls_name == "ExponentialHeightFog":
            log(f"  Fixing ExponentialHeightFog: {label}")
            # Try different property names for the fog component
            comp = None
            for prop_name in ["fog_component", "component", "HeightFogComponent", "FogComponent"]:
                try:
                    comp = actor.get_editor_property(prop_name)
                    if comp:
                        log(f"    + Found fog component as: {prop_name}")
                        break
                except:
                    pass
            
            if not comp:
                # Try getting the component via the actor's components
                try:
                    comps = actor.get_components()
                    for c in comps:
                        if "fog" in c.get_class().get_name().lower():
                            comp = c
                            log(f"    + Found fog component via get_components: {c.get_class().get_name()}")
                            break
                except:
                    pass
            
            if comp:
                try:
                    comp.set_editor_property("fog_density", 0.015)
                    log("    + fog_density = 0.015")
                except Exception as e:
                    log(f"    ! fog_density failed: {e}", "WARN")
                try:
                    comp.set_editor_property("fog_max_opacity", 0.75)
                    log("    + fog_max_opacity = 0.75")
                except:
                    pass
                try:
                    comp.set_editor_property("start_distance", 300.0)
                    log("    + start_distance = 300")
                except:
                    pass
                try:
                    comp.set_editor_property("volumetric_fog_enable", True)
                    log("    + volumetric_fog_enable = True")
                except:
                    log("    ! volumetric_fog not available", "WARN")
            else:
                log("    ! Could not find fog component", "WARN")
    except:
        pass

# Take final screenshot
log("Taking final screenshot...")
try:
    unreal.AutomationLibrary.take_high_res_screenshot(
        1920, 1080,
        "C:/Users/Tap/Desktop/Stage1_Final_Lighting.png"
    )
    log("  + Screenshot saved!")
except Exception as e:
    log(f"  ! Screenshot failed: {e}", "ERROR")

log("=" * 60)
log("FINAL LIGHTING FIX COMPLETE")
log("=" * 60)
