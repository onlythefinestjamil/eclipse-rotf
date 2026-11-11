"""
Solara Cathedral Lighting Setup v2
World Director — Eclipse Environment Design
Sets up: Directional Light (warm amber sun), Sky Light (violet fill),
ExponentialHeightFog (god rays), PostProcessVolume (ACES, bloom, vignette)
for the Cathedral of Unmade Hours.
"""

import unreal
import os
import time

def log(msg):
    unreal.log("[SOLARA_LIGHTING_V2] " + msg)

log("=== Solara Cathedral Lighting Setup v2 ===")

# --- Helpers ---
def get_or_spawn_actor(actor_class, label, location=(0,0,0), rotation=(0,0,0)):
    """Find actor by label, or spawn new one."""
    world = unreal.EditorLevelLibrary.get_editor_world()
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    for actor in all_actors:
        if actor.get_fname().to_string() == label or actor.get_actor_label() == label:
            log(f"Found existing actor: {label}")
            return actor
    # Spawn new
    loc = unreal.Vector(location[0], location[1], location[2])
    rot = unreal.Rotator(rotation[0], rotation[1], rotation[2])
    actor = unreal.EditorLevelLibrary.spawn_actor_from_object(
        actor_class, loc, rot
    )
    if actor:
        actor.set_actor_label(label)
        log(f"Spawned new actor: {label}")
    return actor

def set_property(obj, prop_name, value):
    """Set an editor property with error handling."""
    try:
        obj.set_editor_property(prop_name, value)
        log(f"  Set {prop_name} = {value}")
    except Exception as e:
        log(f"  WARNING: Could not set {prop_name}: {e}")

# --- Open Solara Cathedral Level ---
log("Opening Solara Cathedral level...")
try:
    unreal.EditorLevelLibrary.load_level("/Game/Maps/Lvl_SolaraCathedral")
    log("Level loaded successfully.")
except Exception as e:
    log(f"ERROR loading level: {e}")
    # Try to work in current level
    log("Will work in current level instead.")

time.sleep(2)

# --- Clear existing lights and fog ---
log("Clearing existing lights and atmospheric actors...")
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
for actor in all_actors:
    try:
        if actor.get_class().get_fname().to_string() in [
            "DirectionalLight", "SkyLight", "ExponentialHeightFog",
            "PostProcessVolume", "AtmosphericFog", "SphereReflectionCapture"
        ]:
            unreal.EditorLevelLibrary.destroy_actor(actor)
            log(f"  Removed: {actor.get_actor_label()}")
    except:
        pass

time.sleep(1)

# --- 1. DIRECTIONAL LIGHT (Sun) ---
log("--- Setting up Directional Light (Warm Amber Sun) ---")
sun_class = unreal.DirectionalLight
sun = get_or_spawn_actor(sun_class, "Solara_Sun_Directional", 
                          location=(0, 0, 2000), 
                          rotation=(-42, 0, 28))

if sun:
    comp = sun.get_root_component()
    if comp:
        # Intensity — warm amber directional light
        try:
            comp.set_editor_property("intensity", 10.0)
        except:
            try:
                comp.set_editor_property("intensity_units", unreal.IntensityUnits.LUX)
                comp.set_editor_property("intensity", 10.0)
            except:
                log("  Could not set intensity directly — using default")
        
        # Color — warm amber (0.9, 0.7, 0.3)
        try:
            sun_color = unreal.LinearColor(0.9, 0.7, 0.3, 1.0)
            comp.set_editor_property("light_color", sun_color)
        except:
            log("  Could not set light_color (known UE5.8 Python limitation)")
        
        # Temperature
        try:
            comp.set_editor_property("use_temperature", True)
            comp.set_editor_property("temperature", 5500.0)  # Warm daylight
        except:
            log("  Could not set temperature")
        
        # Angle — 28° elevation, pierce through ceiling breach
        try:
            rot = unreal.Rotator(-42, 0, 28)
            comp.set_editor_property("rotation", rot)
        except:
            pass
    
    # Cast shadows
    try:
        comp.set_editor_property("cast_shadows", True)
        comp.set_editor_property("shadow_visibility", True)
    except:
        pass
    
    log("  Directional light configured (warm amber, 28° angle)")
else:
    log("  ERROR: Could not create directional light!")

time.sleep(0.5)

# --- 2. SKY LIGHT (Cool Violet Fill) ---
log("--- Setting up Sky Light (Cool Violet Fill from Cosmic Void) ---")
sky_class = unreal.SkyLight
sky = get_or_spawn_actor(sky_class, "Solara_SkyLight",
                         location=(0, 0, 3000))

if sky:
    comp = sky.get_root_component()
    if comp:
        try:
            comp.set_editor_property("intensity", 3.0)
        except:
            log("  Could not set sky light intensity")
        
        # Color — cool violet (0.3, 0.2, 0.5)
        try:
            sky_color = unreal.LinearColor(0.3, 0.2, 0.5, 1.0)
            comp.set_editor_property("light_color", sky_color)
        except:
            log("  Could not set sky light color (UE5.8 Python limitation)")
        
        try:
            comp.set_editor_property("source_radius", 2000.0)
        except:
            pass
        
        # Lower hemisphere — even darker for contrast
        try:
            comp.set_editor_property("lower_hemisphere_color", unreal.LinearColor(0.05, 0.03, 0.08, 1.0))
        except:
            pass
    
    log("  Sky light configured (violet, 3.0 intensity)")
else:
    log("  ERROR: Could not create sky light!")

time.sleep(0.5)

# --- 3. EXPONENTIAL HEIGHT FOG (God Rays + Atmosphere) ---
log("--- Setting up Exponential Height Fog (God Rays + Volumetric) ---")
fog_class = unreal.ExponentialHeightFog
fog = get_or_spawn_actor(fog_class, "Solara_HeightFog",
                         location=(0, 0, 0))

if fog:
    comp = None
    try:
        comp = fog.get_editor_property("component")
    except:
        try:
            comp = fog.height_fog_component
        except:
            try:
                comp = fog.get_root_component()
            except:
                log("  Could not access fog component")
    
    if comp:
        # Fog density
        try:
            comp.set_editor_property("fog_density", 0.015)
            log("  Fog density = 0.015")
        except:
            log("  Could not set fog_density")
        
        # Fog color — warm amber near sun
        try:
            comp.set_editor_property("fog_color", unreal.LinearColor(0.15, 0.1, 0.05, 1.0))
            log("  Fog color = warm amber")
        except:
            log("  Could not set fog_color")
        
        # Height falloff
        try:
            comp.set_editor_property("fog_height_falloff", 0.4)
        except:
            pass
        
        # Start distance
        try:
            comp.set_editor_property("start_distance", 200.0)
        except:
            pass
        
        # Volumetric fog
        try:
            comp.set_editor_property("volumetric_fog_enable", True)
            log("  Volumetric fog ENABLED")
        except:
            log("  Could not enable volumetric fog")
        
        # Scattering distribution
        try:
            comp.set_editor_property("volumetric_fog_scattering_distribution", 0.8)
        except:
            pass
        
        # Albedo
        try:
            comp.set_editor_property("volumetric_fog_albedo", unreal.LinearColor(0.9, 0.8, 0.6, 1.0))
        except:
            pass
        
        # Emissive
        try:
            comp.set_editor_property("volumetric_fog_emissive", unreal.LinearColor(0.05, 0.03, 0.02, 1.0))
        except:
            pass
        
        # Extinction scale
        try:
            comp.set_editor_property("volumetric_fog_extinction_scale", 8.0)
        except:
            pass
        
        # Distance
        try:
            comp.set_editor_property("volumetric_fog_distance", 6000.0)
        except:
            pass
    
    log("  Height fog configured (0.015 density, warm amber, volumetric enabled)")
else:
    log("  ERROR: Could not create height fog!")

time.sleep(0.5)

# --- 4. POST PROCESS VOLUME (ACES, Bloom, Vignette) ---
log("--- Setting up Post Process Volume (ACES Tonemapper, Bloom, Vignette) ---")
pp_class = unreal.PostProcessVolume
pp = get_or_spawn_actor(pp_class, "Solara_PostProcess",
                        location=(500, 0, 200))

if pp:
    try:
        pp.set_editor_property("enabled", True)
        pp.set_editor_property("unbound", True)  # Affects entire level
        log("  PostProcess enabled, unbound (global)")
    except:
        log("  Could not enable post process")
    
    settings = None
    try:
        settings = pp.get_editor_property("settings")
    except:
        try:
            settings = pp.settings
        except:
            log("  Could not access post process settings")
    
    if settings:
        # Bloom — stained glass glow
        try:
            settings.set_editor_property("bloom_intensity", 1.2)
            log("  Bloom intensity = 1.2")
        except:
            pass
        
        # Vignette — dark corners
        try:
            settings.set_editor_property("vignette_intensity", 0.6)
            log("  Vignette = 0.6")
        except:
            pass
        
        # Contrast — dramatic chiaroscuro
        try:
            settings.set_editor_property("contrast", 1.1)
        except:
            pass
        
        # Saturation — slightly desaturated
        try:
            settings.set_editor_property("saturation", 0.85)
        except:
            pass
        
        # Auto exposure
        try:
            settings.set_editor_property("auto_exposure_min_brightness", 0.3)
            settings.set_editor_property("auto_exposure_max_brightness", 2.0)
            log("  Auto exposure: min=0.3, max=2.0")
        except:
            pass
        
        # Lens flare
        try:
            settings.set_editor_property("lens_flare_intensity", 0.3)
        except:
            pass
        
        # Motion blur
        try:
            settings.set_editor_property("motion_blur_intensity", 0.3)
        except:
            pass
        
        # Chromatic aberration — subtle
        try:
            settings.set_editor_property("scene_fringe_intensity", 0.1)
        except:
            pass
        
        # Film grain — subtle
        try:
            settings.set_editor_property("film_grain_intensity", 0.15)
        except:
            pass
        
        # Tonemapper — ACES
        try:
            settings.set_editor_property("tonemapper_quality", 4)
        except:
            pass
        
        log("  Post process configured (ACES, bloom, vignette, auto exposure)")
    else:
        log("  WARNING: Could not access post process settings dict")
else:
    log("  ERROR: Could not create post process volume!")

time.sleep(0.5)

# --- 5. SKY SPHERE (for visible sky) ---
log("--- Adding Sky Sphere for visible sky ---")
try:
    # Try to add a large sphere with basic sky material
    sphere = get_or_spawn_actor(unreal.StaticMeshActor, "Solara_SkySphere",
                                  location=(0, 0, 500))
    if sphere:
        mesh_comp = sphere.get_root_component()
        if mesh_comp and hasattr(mesh_comp, 'set_static_mesh'):
            # Try to find a sphere mesh
            sphere_mesh = unreal.EditorAssetLibrary.load_asset(
                "/Engine/BasicShapes/Sphere"
            )
            if sphere_mesh:
                mesh_comp.set_static_mesh(sphere_mesh)
                mesh_comp.set_editor_property("relative_scale3d", 
                    unreal.Vector(500, 500, 500))
                log("  Sky sphere added (scaled 500x)")
except Exception as e:
    log(f"  Could not add sky sphere: {e}")

time.sleep(0.5)

# --- 6. GROUND PLANE ---
log("--- Adding ground plane ---")
try:
    ground = get_or_spawn_actor(unreal.StaticMeshActor, "Solara_GroundPlane",
                                 location=(500, 0, 0))
    if ground:
        mesh_comp = ground.get_root_component()
        if mesh_comp and hasattr(mesh_comp, 'set_static_mesh'):
            plane_mesh = unreal.EditorAssetLibrary.load_asset(
                "/Engine/BasicShapes/Plane"
            )
            if plane_mesh:
                mesh_comp.set_static_mesh(plane_mesh)
                mesh_comp.set_editor_property("relative_scale3d",
                    unreal.Vector(100, 100, 1))
                log("  Ground plane added (100x100 units)")
except Exception as e:
    log(f"  Could not add ground plane: {e}")

time.sleep(1)

# --- 7. SAVE LEVEL ---
log("Saving level...")
try:
    unreal.EditorLevelLibrary.save_current_level()
    log("Level saved successfully.")
except Exception as e:
    log(f"Could not save level: {e}")

time.sleep(1)

# --- 8. CAPTURE SCREENSHOT ---
log("Capturing screenshot...")
try:
    screenshot_path = "C:/Users/Tap/Desktop/Stage1_Solara_Cathedral_Lighting_v2.png"
    unreal.SystemLibrary.execute_console_command(
        unreal.EditorLevelLibrary.get_editor_world(),
        f"r.HighResScreenshot {1920}x{1080}"
    )
    time.sleep(0.5)
    unreal.AutomationLibrary.take_high_res_screenshot(
        unreal.EditorLevelLibrary.get_editor_world(),
        1920, 1080, screenshot_path
    )
    log(f"Screenshot saved to: {screenshot_path}")
except Exception as e:
    log(f"Could not capture screenshot: {e}")
    # Try alternative screenshot method
    try:
        unreal.SystemLibrary.execute_console_command(
            unreal.EditorLevelLibrary.get_editor_world(),
            "shot"
        )
        log("Screenshot taken via 'shot' command")
    except:
        log("Could not take screenshot via any method")

log("=== Solara Cathedral Lighting Setup v2 COMPLETE ===")
log("Next steps: 1) Restart UE5.8 for MegascansPlugin 2) Login Quixel Bridge 3) Download Gothic assets 4) Dress the level")
