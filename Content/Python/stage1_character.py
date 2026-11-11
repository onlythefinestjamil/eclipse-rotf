"""
Eclipse Stage 1 — JUST the Aetherborn character model.
Import the best AAA Aetherborn GLB and place it in the level with proper lighting.
Run inside UE5.8:  py C:/Users/Tap/EclipseRotF/Content/Python/stage1_character.py
"""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_character.log")

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
log("ECLIPSE STAGE 1 — Aetherborn Character Model")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

# Step 1: Import the best AAA Aetherborn model
AETHERBORN_GLB = r"C:\Users\Tap\EclipseRotF\Content\Characters\AETHERBORN_HERO_ALT_AAA.glb"
IMPORT_PATH = "/Game/Characters/AETHERBORN_HERO_AAA"

log(f"1. Importing Aetherborn AAA from {AETHERBORN_GLB}...")

try:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", AETHERBORN_GLB)
    task.set_editor_property("destination_path", "/Game/Characters/")
    task.set_editor_property("destination_name", "AETHERBORN_HERO_AAA")
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.import_asset_tasks([task])
    log("  + Import task submitted!")
except Exception as e:
    log(f"  ! Import task failed: {e}", "WARN")

# Wait for import
import time
log("  Waiting 10s for import...")
time.sleep(10)

# Step 2: Find the imported StaticMesh
log("2. Finding imported Aetherborn StaticMesh...")
MESH_PATH = "/Game/Characters/AETHERBORN_HERO_AAA/StaticMeshes/AETHERBORN_HERO_AAA.AETHERBORN_HERO_AAA"
aetherborn_mesh = EAL.load_asset(MESH_PATH)

if not aetherborn_mesh:
    log("  ! Not at expected path, searching...")
    assets = EAL.list_assets("/Game/Characters/AETHERBORN_HERO_AAA/", recursive=True, include_folder=False)
    log(f"  Found {len(assets)} assets in AETHERBORN_HERO_AAA:")
    for a in assets:
        log(f"    - {a}")
        loaded = EAL.load_asset(a)
        if loaded and "StaticMesh" in loaded.get_class().get_name():
            aetherborn_mesh = loaded
            MESH_PATH = a
            log(f"  + Found StaticMesh at: {a}")
            break

if not aetherborn_mesh:
    # Try alternate naming
    assets2 = EAL.list_assets("/Game/Characters/", recursive=False, include_folder=False)
    for a in assets2:
        if "AETHERBORN_HERO" in a.upper() or "Aetherborn_Hero" in a:
            loaded = EAL.load_asset(a)
            if loaded:
                log(f"  Trying: {a} ({loaded.get_class().get_name()})")
                if "StaticMesh" in loaded.get_class().get_name():
                    aetherborn_mesh = loaded
                    MESH_PATH = a
                    log(f"  + Found at: {a}")
                    break

# Step 3: Remove old player and spawn new one
log("3. Setting up Aetherborn in level...")

# Remove old Aetherborn actors
for actor in ELL.get_all_level_actors():
    try:
        label = actor.get_actor_label()
        if "Aetherborn" in label or "Stage1_Player" in label:
            actor.destroy_actor()
            log(f"  - Removed old: {label}")
    except:
        pass

# Spawn new Aetherborn
if aetherborn_mesh:
    log(f"  + Aetherborn mesh loaded: {aetherborn_mesh.get_name()}")
    log(f"  + Type: {aetherborn_mesh.get_class().get_name()}")
    
    actor = ELL.spawn_actor_from_class(unreal.StaticMeshActor, Vec(0, 0, 0), Rot(0, 0, 0))
    if actor:
        actor.set_actor_label("Aetherborn_Hero_AAA")
        comp = actor.get_editor_property("static_mesh_component")
        if comp:
            comp.set_static_mesh(aetherborn_mesh)
            try:
                comp.set_editor_property("cast_shadow", True)
            except:
                pass
        # Scale - AAA models from Meshy can be different scales
        # Try scale 1 first, adjust if needed
        actor.set_actor_scale3d(Vec(1, 1, 1))
        log(f"  + Aetherborn spawned at origin!")
        
        # Check the mesh bounds to see if it needs scaling
        try:
            bounds = comp.get_local_bounds()
            log(f"  + Mesh bounds: min={bounds[0]}, max={bounds[1]}")
        except:
            pass
else:
    log("  ! Could not find Aetherborn mesh - trying existing Aetherborn_male_HQ")
    existing = EAL.load_asset("/Game/Characters/Aetherborn_male_HQ/StaticMeshes/Aetherborn_male_HQ.Aetherborn_male_HQ")
    if existing:
        actor = ELL.spawn_actor_from_class(unreal.StaticMeshActor, Vec(0, 0, 0), Rot(0, 0, 0))
        if actor:
            actor.set_actor_label("Aetherborn_Hero")
            comp = actor.get_editor_property("static_mesh_component")
            if comp:
                comp.set_static_mesh(existing)
            log("  + Spawned existing Aetherborn_male_HQ as fallback")

# Step 4: Simple lighting — just enough to see the character
log("4. Setting up character lighting...")

# Remove all existing lights
for actor in ELL.get_all_level_actors():
    try:
        cls = actor.get_class().get_name()
        if cls in ("DirectionalLight", "SkyLight", "PointLight", "SpotLight", "ExponentialHeightFog"):
            actor.destroy_actor()
            log(f"  - Removed: {actor.get_actor_label()}")
    except:
        pass

# Add one clean directional light
sun = ELL.spawn_actor_from_class(unreal.DirectionalLight, Vec(500, -500, 2000), Rot(-45, 45, 0))
if sun:
    sun.set_actor_label("Character_Sun")
    comp = sun.get_editor_property("light_component")
    if comp:
        comp.set_editor_property("intensity", 10.0)
    log("  + Sun: intensity=10, angle=-45deg")

# Add sky light for fill
sky = ELL.spawn_actor_from_class(unreal.SkyLight, Vec(0, 0, 3000), Rot(0, 0, 0))
if sky:
    sky.set_actor_label("Character_SkyLight")
    comp = sky.get_editor_property("light_component")
    if comp:
        comp.set_editor_property("intensity", 3.0)
    log("  + SkyLight: intensity=3")

# Add rim light from behind
rim = ELL.spawn_actor_from_class(unreal.PointLight, Vec(-300, 300, 500), Rot(0, 0, 0))
if rim:
    rim.set_actor_label("Character_RimLight")
    comp = rim.get_editor_property("light_component")
    if comp:
        comp.set_editor_property("intensity", 5000.0)
        comp.set_editor_property("attenuation_radius", 2000.0)
    log("  + RimLight: intensity=5000")

# Step 5: List all actors
log("5. All actors in level:")
for a in ELL.get_all_level_actors():
    try:
        loc = a.get_actor_location()
        log(f"  - {a.get_actor_label()} ({a.get_class().get_name()}) @ ({int(loc.x)}, {int(loc.y)}, {int(loc.z)})")
    except:
        pass

# Step 6: Take screenshot
log("6. Taking screenshot...")
try:
    unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "C:/Users/Tap/Desktop/Stage1_Aetherborn_Character.png")
    log("  + Screenshot saved!")
except Exception as e:
    log(f"  ! Screenshot failed: {e}", "ERROR")

log("=" * 60)
log("AETHERBORN CHARACTER COMPLETE")
log("=" * 60)
