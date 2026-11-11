"""
Eclipse Stage 1 — Refined Cathedral Scene
Based on motion clip reference: warm but not orange, volumetric god rays,
gothic atmosphere with floating dust/embers.
Run inside UE5.8:  py C:/Users/Tap/EclipseRotF/Content/Python/stage1_refined.py
"""
import os
import time
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_refined.log")

# Clear old log
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
    log("ERROR: unreal module not available!")
    raise

log("=" * 60)
log("ECLIPSE STAGE 1 — Refined Cathedral Scene")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary
AL = unreal.AutomationLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

# ============================================================================
# 1. REMOVE DUPLICATES — keep only one of each actor type
# ============================================================================
log("1. Removing duplicate actors...")
all_actors = ELL.get_all_level_actors()
log(f"  Total actors before cleanup: {len(all_actors)}")

# We want to keep: 1 cathedral, 1 player, 1 enemy, 1 directional light,
# 1 sky light, 1 fog, 1 sky sphere. Remove everything else.
keep_labels = set()
removed = 0

# First pass: identify what to keep
cathedral_kept = False
player_kept = False
enemy_kept = False
dir_light_kept = False
sky_light_kept = False
fog_kept = False
sky_sphere_kept = False

actors_to_destroy = []

for actor in all_actors:
    try:
        label = actor.get_actor_label()
        cls = actor.get_class().get_name()

        # Skip non-actor or utility actors
        if cls in ("WorldSettings", "LevelScriptActor", "PlayerStart", "CineCameraActor"):
            continue

        # Cathedral environment — keep first only
        if cls == "StaticMeshActor" and "Cathedral" in label:
            if cathedral_kept:
                actors_to_destroy.append((actor, label, "duplicate cathedral"))
            else:
                cathedral_kept = True
                log(f"  KEEP: {label}")
            continue

        # Player Aetherborn — keep first only
        if cls == "StaticMeshActor" and ("Aetherborn" in label or "Stage1_Player" in label):
            if player_kept:
                actors_to_destroy.append((actor, label, "duplicate player"))
            else:
                player_kept = True
                log(f"  KEEP: {label}")
            continue

        # Enemy Berserker — keep first only
        if cls == "StaticMeshActor" and ("Berserker" in label or "Stage1_Enemy" in label):
            if enemy_kept:
                actors_to_destroy.append((actor, label, "duplicate enemy"))
            else:
                enemy_kept = True
                log(f"  KEEP: {label}")
            continue

        # Directional light — keep first only
        if cls == "DirectionalLight":
            if dir_light_kept:
                actors_to_destroy.append((actor, label, "extra directional light"))
            else:
                dir_light_kept = True
                log(f"  KEEP: {label}")
            continue

        # Sky light — keep first only
        if cls == "SkyLight":
            if sky_light_kept:
                actors_to_destroy.append((actor, label, "extra sky light"))
            else:
                sky_light_kept = True
                log(f"  KEEP: {label}")
            continue

        # Exponential height fog — keep first only
        if cls == "ExponentialHeightFog":
            if fog_kept:
                actors_to_destroy.append((actor, label, "extra fog"))
            else:
                fog_kept = True
                log(f"  KEEP: {label}")
            continue

        # Sky sphere — keep
        if "Sky_Sphere" in cls or "BP_Sky" in cls or "SkySphere" in cls:
            if sky_sphere_kept:
                actors_to_destroy.append((actor, label, "extra sky sphere"))
            else:
                sky_sphere_kept = True
                log(f"  KEEP: {label}")
            continue

        # Niagara actors from previous runs — remove duplicates
        if cls == "NiagaraActor" and "Atmospheric" in label:
            actors_to_destroy.append((actor, label, "old particle actor"))

        # Remove extra PointLights (rim lights from character setup not needed)
        if cls == "PointLight":
            actors_to_destroy.append((actor, label, "extra point light"))

        # Remove any CineCamera from previous runs
        if cls == "CineCameraActor":
            actors_to_destroy.append((actor, label, "old cine camera"))

    except:
        pass

# Destroy marked actors
for actor, label, reason in actors_to_destroy:
    try:
        actor.destroy_actor()
        removed += 1
        log(f"  - Removed {label} ({reason})")
    except:
        pass

log(f"  Total removed: {removed}")

# ============================================================================
# 2. POSITION CATHEDRAL ENVIRONMENT
# ============================================================================
log("2. Positioning cathedral environment...")
cathedral_found = False
for actor in ELL.get_all_level_actors():
    try:
        label = actor.get_actor_label()
        cls = actor.get_class().get_name()
        if cls == "StaticMeshActor" and "Cathedral" in label:
            cathedral_found = True
            try:
                actor.set_actor_location(Vec(0, 0, 0), False, False)
            except:
                try:
                    actor.set_actor_location(Vec(0, 0, 0))
                except:
                    pass
            try:
                actor.set_actor_rotation(Rot(0, 0, 0), False)
            except:
                try:
                    actor.set_actor_rotation(Rot(0, 0, 0))
                except:
                    pass
            try:
                actor.set_actor_scale3d(Vec(10, 10, 10))
            except:
                pass
            log(f"  + {label} at origin, scale 10x10x10")
            # Ensure mesh is set
            comp = actor.get_editor_property("static_mesh_component")
            if comp:
                try:
                    comp.set_editor_property("cast_shadow", True)
                except:
                    pass
    except:
        pass

# If no cathedral found, spawn one
if not cathedral_found:
    log("  ! No cathedral found — spawning one...")
    try:
        CATHEDRAL_MESH_PATH = "/Game/Characters/SOLARA_CATHEDRAL_AAA/StaticMeshes/SOLARA_CATHEDRAL_AAA.SOLARA_CATHEDRAL_AAA"
        cathedral_mesh = EAL.load_asset(CATHEDRAL_MESH_PATH)
        if cathedral_mesh:
            cath_actor = ELL.spawn_actor_from_class(unreal.StaticMeshActor, Vec(0, 0, 0), Rot(0, 0, 0))
            if cath_actor:
                cath_actor.set_actor_label("Solara_Cathedral_Environment")
                comp = cath_actor.get_editor_property("static_mesh_component")
                if comp:
                    try:
                        comp.set_static_mesh(cathedral_mesh)
                    except:
                        pass
                    try:
                        comp.set_editor_property("cast_shadow", True)
                    except:
                        pass
                try:
                    cath_actor.set_actor_scale3d(Vec(10, 10, 10))
                except:
                    pass
                log("  + Cathedral spawned at origin, scale 10x")
            else:
                log("  ! Could not spawn cathedral actor", "ERROR")
        else:
            log("  ! Could not load cathedral mesh", "ERROR")
    except Exception as e:
        log(f"  ! Cathedral spawn failed: {e}", "ERROR")

# ============================================================================
# 3. POSITION AETHERBORN PLAYER INSIDE CATHEDRAL
# ============================================================================
log("3. Positioning Aetherborn player inside cathedral...")
for actor in ELL.get_all_level_actors():
    try:
        label = actor.get_actor_label()
        if "Aetherborn" in label or "Stage1_Player" in label:
            # Place in front of the cathedral facade, on the ground
            # Cathedral facade is at X≈0, camera is at X≈1100 looking towards -X
            try:
                actor.set_actor_location(Vec(400, 50, 90), False, False)
            except:
                try:
                    actor.set_actor_location(Vec(400, 50, 90))
                except:
                    pass
            try:
                actor.set_actor_rotation(Rot(0, 180, 0), False)
            except:
                try:
                    actor.set_actor_rotation(Rot(0, 180, 0))
                except:
                    pass
            try:
                actor.set_actor_scale3d(Vec(1, 1, 1))
            except:
                pass
            log(f"  + {label} at (400, 50, 90) — in front of cathedral, facing camera")
    except:
        pass

# ============================================================================
# 4. POSITION BERSERKER ENEMY INSIDE CATHEDRAL AT A DISTANCE
# ============================================================================
log("4. Positioning Berserker enemy inside cathedral...")
for actor in ELL.get_all_level_actors():
    try:
        label = actor.get_actor_label()
        if "Berserker" in label or "Stage1_Enemy" in label:
            # Place further in front, at an angle — creates dramatic confrontation
            try:
                actor.set_actor_location(Vec(550, -350, 90), False, False)
            except:
                try:
                    actor.set_actor_location(Vec(550, -350, 90))
                except:
                    pass
            try:
                actor.set_actor_rotation(Rot(0, 160, 0), False)
            except:
                try:
                    actor.set_actor_rotation(Rot(0, 160, 0))
                except:
                    pass
            try:
                actor.set_actor_scale3d(Vec(1, 1, 1))
            except:
                pass
            log(f"  + {label} at (550, -350, 90) — in front of cathedral, facing player")
    except:
        pass

# ============================================================================
# 5. ADJUST LIGHTING — Solara Cathedral Aesthetic
#    Warm but not orange, volumetric god rays, gothic atmosphere
# ============================================================================
log("5. Adjusting lighting for Solara Cathedral aesthetic...")

fog_found = False

for actor in ELL.get_all_level_actors():
    try:
        cls = actor.get_class().get_name()
        label = actor.get_actor_label()

        # --- DIRECTIONAL LIGHT (the sun) ---
        if cls == "DirectionalLight":
            log(f"  Configuring DirectionalLight: {label}")
            comp = actor.get_editor_property("light_component")
            if comp:
                # Intensity 5.0 as specified
                try:
                    comp.set_editor_property("intensity", 5.0)
                except:
                    pass
                # Warm white — NOT orange. Slight golden tint from motion clips
                try:
                    comp.set_editor_property("light_color", unreal.LinearColor(1.0, 0.96, 0.88, 1.0))
                except:
                    try:
                        comp.set_editor_property("light_color", (255, 245, 224, 255))
                    except:
                        pass
                # Shadows
                try:
                    comp.set_editor_property("cast_shadows", True)
                except:
                    pass
                # Volumetric scattering for god rays
                try:
                    comp.set_editor_property("use_volume_scattering", True)
                except:
                    pass
                try:
                    comp.set_editor_property("volumetric_scattering_intensity", 2.0)
                except:
                    pass
                # Shadow settings for soft gothic shadows
                try:
                    comp.set_editor_property("shadow_bias", 0.5)
                except:
                    pass
            else:
                log("  ! light_component not found on directional light", "WARN")
            # Angle: sun coming through cathedral windows at ~35 degrees
            # Wrapped in try/except to prevent silent failure
            try:
                actor.set_actor_rotation(Rot(-38, 135, 0), False)
            except Exception as e:
                try:
                    actor.set_actor_rotation(Rot(-38, 135, 0))
                except Exception as e2:
                    log(f"  ! set_actor_rotation failed: {e2}", "WARN")
            try:
                actor.set_actor_location(Vec(0, -1000, 4000), False, False)
            except:
                try:
                    actor.set_actor_location(Vec(0, -1000, 4000))
                except:
                    pass
            log(f"  + {label}: intensity=5.0, warm white, volumetric god rays")

        # --- SKY LIGHT ---
        elif cls == "SkyLight":
            log(f"  Configuring SkyLight: {label}")
            comp = actor.get_editor_property("light_component")
            if comp:
                # Intensity 2.0 as specified
                try:
                    comp.set_editor_property("intensity", 2.0)
                except:
                    pass
                # Slightly cool fill to balance warm directional
                try:
                    comp.set_editor_property("light_color", unreal.LinearColor(0.75, 0.78, 0.95, 1.0))
                except:
                    try:
                        comp.set_editor_property("light_color", (191, 199, 242, 255))
                    except:
                        pass
            log(f"  + {label}: intensity=2.0, cool ambient fill")

        # --- EXPONENTIAL HEIGHT FOG ---
        elif cls == "ExponentialHeightFog":
            fog_found = True
            log(f"  Configuring ExponentialHeightFog: {label}")
            # Use 'component' NOT 'fog_component' (per UE5.8)
            comp = actor.get_editor_property("component")
            if comp:
                # Moderate density for gothic atmosphere
                try:
                    comp.set_editor_property("fog_density", 0.012)
                except:
                    pass
                try:
                    comp.set_editor_property("fog_max_opacity", 0.45)
                except:
                    pass
                try:
                    comp.set_editor_property("start_distance", 200.0)
                except:
                    pass
                # Enable volumetric fog for god ray scattering
                try:
                    comp.set_editor_property("volumetric_fog_enable", True)
                except:
                    pass
                try:
                    comp.set_editor_property("volumetric_fog_distance", 8000)
                except:
                    pass
                # Warm inscattering — like dusty air in a sunlit cathedral
                try:
                    comp.set_editor_property("fog_inscattering_color", unreal.LinearColor(0.85, 0.75, 0.55, 1.0))
                except:
                    pass
            log(f"  + {label}: density=0.012, opacity=0.45, volumetric=True, warm inscattering")

    except:
        pass

# Spawn ExponentialHeightFog if none exists
if not fog_found:
    log("  No ExponentialHeightFog found — spawning one...")
    try:
        fog_actor = ELL.spawn_actor_from_class(
            unreal.ExponentialHeightFog,
            Vec(0, 0, 0),
            Rot(0, 0, 0)
        )
        if fog_actor:
            fog_actor.set_actor_label("Solara_CathedralFog")
            comp = fog_actor.get_editor_property("component")
            if comp:
                try:
                    comp.set_editor_property("fog_density", 0.012)
                except:
                    pass
                try:
                    comp.set_editor_property("fog_max_opacity", 0.45)
                except:
                    pass
                try:
                    comp.set_editor_property("start_distance", 200.0)
                except:
                    pass
                try:
                    comp.set_editor_property("volumetric_fog_enable", True)
                except:
                    pass
                try:
                    comp.set_editor_property("volumetric_fog_distance", 8000)
                except:
                    pass
                try:
                    comp.set_editor_property("fog_inscattering_color", unreal.LinearColor(0.85, 0.75, 0.55, 1.0))
                except:
                    pass
            log("  + ExponentialHeightFog spawned: density=0.012, opacity=0.45, volumetric=True")
    except Exception as e:
        log(f"  ! Could not spawn fog: {e}", "WARN")

# ============================================================================
# 6. ADD ATMOSPHERIC PARTICLES — floating dust / embers
# ============================================================================
log("6. Adding atmospheric particles (floating dust/embers)...")

dust_spawned = False

# Try loading built-in Niagara dust systems (skip slow /Game/ recursive scan)
builtin_paths = [
    "/Niagara/Systems/NS_FloatingDust",
    "/Niagara/Systems/NS_Dust",
    "/Niagara/Systems/NS_Embers",
    "/Engine/Art/WorldRecreation/Niagara/Systems/NS_FloatingDust",
    "/Game/Art/Niagara/NS_FloatingDust",
    "/Game/VFX/NS_Dust",
]
dust_system_path = None
for path in builtin_paths:
    try:
        test = EAL.load_asset(path)
        if test:
            dust_system_path = path
            log(f"  Found Niagara system: {path}")
            break
    except:
        pass

# Spawn the Niagara actor if we found a system
if dust_system_path:
    try:
        dust_asset = EAL.load_asset(dust_system_path)
        if dust_asset:
            dust_actor = ELL.spawn_actor_from_class(
                unreal.NiagaraActor,
                Vec(600, 300, 300),
                Rot(0, 0, 0)
            )
            if dust_actor:
                dust_actor.set_actor_label("Stage1_AtmosphericDust")
                dust_actor.set_actor_scale3d(Vec(12, 12, 6))
                niagara_comp = dust_actor.get_editor_property("niagara_component")
                if niagara_comp:
                    try:
                        niagara_comp.set_editor_property("asset", dust_asset)
                    except:
                        pass
                log(f"  + Atmospheric dust spawned with {dust_system_path}")
                dust_spawned = True
    except Exception as e:
        log(f"  ! Niagara spawn failed: {e}", "WARN")

if not dust_spawned:
    # Volumetric fog + height fog provides the atmospheric dust look even without particles
    log("  ! No Niagara dust system found — volumetric fog provides atmospheric scattering", "WARN")
    # Try a quick non-recursive Niagara asset check
    try:
        niagara_assets = EAL.list_assets("/Niagara/", recursive=False, include_folder=False)
        if niagara_assets:
            log(f"  /Niagara/ assets: {niagara_assets[:5]}")
    except:
        log("  ! No /Niagara/ content folder", "WARN")

# ============================================================================
# 7. SET CINEMATIC CAMERA ANGLE AND TAKE SCREENSHOT
# ============================================================================
log("7. Setting cinematic camera and taking screenshot...")

# First, remove duplicate CineCameraActors from previous runs
cine_cams = []
for actor in ELL.get_all_level_actors():
    try:
        if actor.get_class().get_name() == "CineCameraActor":
            cine_cams.append(actor)
    except:
        pass
# Keep only the last one, remove the rest
if len(cine_cams) > 1:
    for cam in cine_cams[:-1]:
        try:
            cam.destroy_actor()
            log(f"  - Removed duplicate CineCameraActor")
        except:
            pass

# Get cathedral bounds to position camera inside
# Use the original viewport position which showed the cathedral
cam_loc = Vec(1105, 0, 296)
cam_rot = Rot(-15, -180, 0)
cathedral_bounds = None
for actor in ELL.get_all_level_actors():
    try:
        label = actor.get_actor_label()
        cls = actor.get_class().get_name()
        if cls == "StaticMeshActor" and "Cathedral" in label:
            origin = actor.get_actor_location()
            box_extent = actor.get_actor_scale3d()
            log(f"  Cathedral at ({int(origin.x)}, {int(origin.y)}, {int(origin.z)}), scale ({int(box_extent.x)}, {int(box_extent.y)}, {int(box_extent.z)})")
            # Try to get actual bounds
            try:
                comp = actor.get_editor_property("static_mesh_component")
                if comp:
                    bounds = comp.get_local_bounds()
                    # get_local_bounds() returns a tuple (origin, extent)
                    if isinstance(bounds, tuple) and len(bounds) == 2:
                        bounds_origin, bounds_extent = bounds
                        log(f"  Cathedral mesh bounds: origin=({int(bounds_origin.x)}, {int(bounds_origin.y)}, {int(bounds_origin.z)}), extent=({int(bounds_extent.x)}, {int(bounds_extent.y)}, {int(bounds_extent.z)})")
                        scaled_x = bounds_extent.x * box_extent.x
                        scaled_y = bounds_extent.y * box_extent.y
                        scaled_z = bounds_extent.z * box_extent.z
                        log(f"  Cathedral scaled extent: ({int(scaled_x)}, {int(scaled_y)}, {int(scaled_z)})")
                        # Camera position is set above — don't override here
                    elif hasattr(bounds, 'box_extent'):
                        be = bounds.box_extent
                        scaled_x = be.x * box_extent.x
                        scaled_y = be.y * box_extent.y
                        scaled_z = be.z * box_extent.z
                        log(f"  Cathedral scaled extent: ({int(scaled_x)}, {int(scaled_y)}, {int(scaled_z)})")
                        # Camera position is set above — don't override here
                    else:
                        log(f"  Cathedral bounds format: {type(bounds)} = {bounds}")
            except Exception as e:
                log(f"  ! Could not get bounds: {e}", "WARN")
    except:
        pass

log(f"  Camera target: ({int(cam_loc.x)}, {int(cam_loc.y)}, {int(cam_loc.z)}) rot=({cam_rot.pitch:.0f}, {cam_rot.yaw:.0f}, {cam_rot.roll:.0f})")

# Discover available methods on key classes
log("  Discovering camera APIs...")
try:
    es = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    if es:
        methods = [m for m in dir(es) if not m.startswith('_')]
        cam_methods = [m for m in methods if any(kw in m.lower() for kw in ['camera', 'view', 'viewport', 'screenshot', 'perspective'])]
        log(f"  UnrealEditorSubsystem camera/view methods: {cam_methods}")
except Exception as e:
    log(f"  ! Introspection failed: {e}", "WARN")

# Try AutomationLibrary methods
try:
    al_methods = [m for m in dir(AL) if not m.startswith('_')]
    al_cam = [m for m in al_methods if any(kw in m.lower() for kw in ['camera', 'view', 'screenshot', 'screen', 'render'])]
    log(f"  AutomationLibrary screenshot/render methods: {al_cam}")
except:
    pass

# Try multiple approaches to set the viewport camera
viewport_set = False

# Approach 1: UnrealEditorSubsystem.set_level_viewport_camera_info
# Set the viewport to Lit mode first
try:
    AL.set_editor_viewport_view_mode(0)  # 0 = Lit
    log("  + Viewport set to Lit mode")
except:
    try:
        AL.set_editor_active_viewport_view_mode(0)
        log("  + Viewport set to Lit mode (active)")
    except:
        log("  ! Could not set Lit mode", "WARN")

# Don't change camera position — leave it at current position
# (previous camera set attempts produced black screenshots)
log("  Keeping current viewport camera position (not changing it)")
viewport_set = True  # Skip all camera-setting approaches

# Approach 2: LevelEditorSubsystem with camera info
if not viewport_set:
    try:
        level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        if level_editor:
            # Try set_level_editor_viewport_camera_info
            cam_info = unreal.LevelEditorViewportCameraInfo()
            cam_info.set_view_location(cam_loc)
            cam_info.set_view_rotation(cam_rot)
            try:
                level_editor.set_level_editor_viewport_camera_info(0, cam_info)
                viewport_set = True
                log(f"  + Viewport camera set via LevelEditorSubsystem")
            except Exception as e:
                log(f"  ! LevelEditorSubsystem camera set failed: {e}", "WARN")
    except Exception as e:
        log(f"  ! LevelEditorSubsystem not available: {e}", "WARN")

# Approach 3: EditorLevelLibrary
if not viewport_set:
    try:
        cam_info = unreal.LevelEditorViewportCameraInfo()
        cam_info.set_view_location(cam_loc)
        cam_info.set_view_rotation(cam_rot)
        ELL.set_level_editor_viewport_camera_info(0, cam_info)
        viewport_set = True
        log(f"  + Viewport camera set via EditorLevelLibrary")
    except Exception as e:
        log(f"  ! EditorLevelLibrary camera set failed: {e}", "WARN")

if not viewport_set:
    log("  ! Could not set viewport camera via any method", "WARN")

# Wait for viewport to update (use unreal's frame pump, not time.sleep)
# time.sleep blocks the game thread; instead just proceed to screenshot
log("  Proceeding to screenshot...")

# Force loading completion before screenshot
try:
    AL.finish_loading_before_screenshot()
    log("  + Loading finished before screenshot")
except:
    log("  ! finish_loading_before_screenshot failed", "WARN")

# Take the screenshot — try take_automation_screenshot_at_camera first
screenshot_saved = False
screenshot_path = "C:/Users/Tap/Desktop/Stage1_Refined.png"

# Try take_automation_screenshot_at_camera (takes from a specified camera position)
if not screenshot_saved:
    try:
        # Get the world context object from an actor in the level
        all_actors = ELL.get_all_level_actors()
        world = None
        if all_actors:
            world = all_actors[0].get_world()
        if not world:
            # Try getting world from the subsystem
            try:
                editor_subsys2 = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
                if editor_subsys2:
                    world = editor_subsys2.get_world()
            except:
                pass
        log(f"  World context: {world}")
        if world:
            screenshot_options = AL.get_default_screenshot_options_for_rendering()
            AL.take_automation_screenshot_at_camera(
                world,
                1920, 1080,
                cam_loc, cam_rot,
                screenshot_path
            )
            screenshot_saved = True
            log(f"  + Screenshot saved via take_automation_screenshot_at_camera")
        else:
            log("  ! No world context available", "WARN")
    except Exception as e:
        log(f"  ! take_automation_screenshot_at_camera failed: {e}", "WARN")
        # Try with gameplay options
        try:
            all_actors = ELL.get_all_level_actors()
            world = all_actors[0].get_world() if all_actors else None
            if world:
                screenshot_options = AL.get_default_screenshot_options_for_gameplay()
                AL.take_automation_screenshot_at_camera(
                    world,
                    1920, 1080,
                    cam_loc, cam_rot,
                    screenshot_path
                )
                screenshot_saved = True
                log(f"  + Screenshot saved via take_automation_screenshot_at_camera (gameplay options)")
        except Exception as e2:
            log(f"  ! take_automation_screenshot_at_camera (gameplay) failed: {e2}", "WARN")

# Fall back to take_high_res_screenshot (from current viewport)
if not screenshot_saved:
    try:
        AL.take_high_res_screenshot(
            1920, 1080,
            screenshot_path
        )
        screenshot_saved = True
        log("  + Screenshot saved via take_high_res_screenshot (from viewport)")
    except Exception as e:
        log(f"  ! Screenshot failed: {e}", "ERROR")

if not screenshot_saved:
    log("  ! Could not save screenshot through any method", "ERROR")

# Clean up the cine camera (optional — leave for reference)
# We'll leave it so it doesn't interfere

# ============================================================================
# 8. FINAL ACTOR LIST
# ============================================================================
log("8. Final actor list:")
final_actors = ELL.get_all_level_actors()
log(f"  Total actors: {len(final_actors)}")
for a in final_actors:
    try:
        loc = a.get_actor_location()
        log(f"  - {a.get_actor_label()} ({a.get_class().get_name()}) @ ({int(loc.x)}, {int(loc.y)}, {int(loc.z)})")
    except:
        pass

# ============================================================================
# 9. SAVE THE LEVEL
# ============================================================================
log("9. Saving level...")
saved = False
try:
    ELL.save_current_level()
    saved = True
    log("  + Level saved via EditorLevelLibrary!")
except Exception as e:
    log(f"  ! Save via ELL failed: {e}", "WARN")

if not saved:
    try:
        # Try saving via the asset library
        world = ELL.get_world()
        if world:
            EAL.save_asset(world.get_path_name())
            saved = True
            log("  + Level saved via EditorAssetLibrary!")
    except Exception as e:
        log(f"  ! Save via EAL failed: {e}", "WARN")

if not saved:
    try:
        # Try the unreal.AutomationLibrary save
        unreal.EditorLoadingAndSavingUtils.save_current_level()
        saved = True
        log("  + Level saved via EditorLoadingAndSavingUtils!")
    except:
        pass

if not saved:
    log("  ! Could not save level through any method", "ERROR")

log("=" * 60)
log("STAGE 1 REFINED COMPLETE")
log("=" * 60)
