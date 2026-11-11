"""
Eclipse Stage 1 — Import AAA Cathedral environment model and position it in the level.
Also reposition Aetherborn + Berserker relative to the new cathedral.
Run inside UE5.8:  py C:/Users/Tap/EclipseRotF/Content/Python/stage1_import_cathedral.py
"""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_import_cathedral.log")

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
log("ECLIPSE STAGE 1 — Import AAA Cathedral + Position Everything")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

# 1. Import the cathedral GLB using Interchange or Asset tools
CATHEDRAL_GLB = r"C:\Users\Tap\EclipseRotF\Content\Characters\SOLARA_CATHEDRAL_AAA.glb"
CATHEDRAL_IMPORT_PATH = "/Game/Characters/SOLARA_CATHEDRAL_AAA"

log(f"1. Importing cathedral model from {CATHEDRAL_GLB}...")
log(f"   Target: {CATHEDRAL_IMPORT_PATH}")

try:
    # Use AssetTools to import the GLB
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    # Try Interchange import
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", CATHEDRAL_GLB)
    task.set_editor_property("destination_path", "/Game/Characters/")
    task.set_editor_property("destination_name", "SOLARA_CATHEDRAL_AAA")
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    
    asset_tools.import_asset_tasks([task])
    log("  + Import task submitted!")
    
    # Check if the asset imported
    cathedral_asset = EAL.load_asset(CATHEDRAL_IMPORT_PATH)
    if cathedral_asset:
        log(f"  + Cathedral asset loaded: {cathedral_asset.get_name()}")
        log(f"  + Type: {cathedral_asset.get_class().get_name()}")
    else:
        log("  ! Cathedral asset not found after import, trying alternate path...")
        # Try to find it by searching
        assets = EAL.list_assets("/Game/Characters/", recursive=False, include_folder=False)
        for a in assets:
            if "CATHEDRAL" in a.upper() or "cathedral" in a.lower():
                cathedral_asset = EAL.load_asset(a)
                if cathedral_asset:
                    log(f"  + Found cathedral at: {a}")
                    log(f"  + Type: {cathedral_asset.get_class().get_name()}")
                    break
except Exception as e:
    log(f"  ! Import failed: {e}", "WARN")
    log("  Trying alternate import method...", "WARN")
    
    # Try using the InterchangeManager
    try:
        interchange = unreal.InterchangeManager.get_interchange_manager()
        log(f"  + InterchangeManager found")
    except:
        pass
    
    # Try direct load - maybe Aura already imported it
    cathedral_asset = None
    for path in [
        CATHEDRAL_IMPORT_PATH,
        "/Game/Characters/SOLARA_CATHEDRAL_AAA",
        "/Game/Characters/Solara_Cathedral_AAA",
    ]:
        a = EAL.load_asset(path)
        if a:
            cathedral_asset = a
            log(f"  + Found at {path}")
            break

# 2. Spawn the cathedral in the level
log("2. Spawning cathedral in level...")
try:
    if cathedral_asset:
        # If it's a static mesh, spawn as StaticMeshActor
        cls_name = cathedral_asset.get_class().get_name()
        log(f"  Asset class: {cls_name}")
        
        if "StaticMesh" in cls_name:
            cathedral_actor = ELL.spawn_actor_from_class(
                unreal.StaticMeshActor,
                Vec(0, 0, 0),
                Rot(0, 0, 0),
            )
            if cathedral_actor:
                cathedral_actor.set_actor_label("Solara_Cathedral_AAA")
                comp = cathedral_actor.get_editor_property("static_mesh_component")
                if comp:
                    comp.set_static_mesh(cathedral_asset)
                    comp.set_editor_property("visibility", True)
                    comp.set_editor_property("cast_shadow", True)
                log(f"  + Cathedral spawned at origin!")
        else:
            log(f"  ! Unexpected asset type: {cls_name}", "WARN")
    else:
        log("  ! Cathedral asset not available — skipping spawn", "WARN")
except Exception as e:
    log(f"  ! Spawn failed: {e}", "ERROR")

# 3. Reposition Aetherborn + Berserker relative to cathedral
log("3. Repositioning characters...")
for actor in ELL.get_all_level_actors():
    try:
        label = actor.get_actor_label()
        if "Aetherborn" in label or "Stage1_Player" in label:
            actor.set_actor_location(Vec(0, 0, 50))
            actor.set_actor_rotation(Rot(0, 0, 0))
            log(f"  + {label} at (0, 0, 50)")
        elif "Berserker" in label or "Stage1_Enemy" in label:
            actor.set_actor_location(Vec(800, 800, 50))
            actor.set_actor_rotation(Rot(0, 180, 0))
            log(f"  + {label} at (800, 800, 50)")
    except:
        pass

# 4. List all actors
log("4. All actors in level:")
for a in ELL.get_all_level_actors():
    try:
        loc = a.get_actor_location()
        log(f"  - {a.get_actor_label()} ({a.get_class().get_name()}) @ ({int(loc.x)}, {int(loc.y)}, {int(loc.z)})")
    except:
        pass

# 5. Take screenshot
log("5. Taking screenshot...")
try:
    unreal.AutomationLibrary.take_high_res_screenshot(
        1920, 1080,
        "C:/Users/Tap/Desktop/Stage1_Cathedral_AAA_Imported.png"
    )
    log("  + Screenshot saved!")
except Exception as e:
    log(f"  ! Screenshot failed: {e}", "ERROR")

log("=" * 60)
log("CATHEDRAL IMPORT COMPLETE")
log("=" * 60)
