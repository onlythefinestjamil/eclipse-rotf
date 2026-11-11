"""Import the cathedral GLB using the Interchange pipeline."""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_import_cathedral_v2.log")

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
log("ECLIPSE — Import Cathedral GLB v2")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

CATHEDRAL_GLB = r"C:\Users\Tap\EclipseRotF\Content\Characters\SOLARA_CATHEDRAL_AAA.glb"

# Method 1: Try Interchange Manager
log("Method 1: Interchange Manager...")
try:
    im = unreal.InterchangeManager.get_interchange_manager()
    if im:
        log("  + InterchangeManager found")
        # Import the GLB
        result = im.import_asset_from_file_data(CATHEDRAL_GLB)
        if result:
            log(f"  + Import result: {result}")
        else:
            log("  ! Import returned None", "WARN")
except Exception as e:
    log(f"  ! Interchange failed: {e}", "WARN")

# Method 2: Try AssetImportTask with GLTF settings
log("Method 2: AssetImportTask with GLTF...")
try:
    import_tasks = []
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", CATHEDRAL_GLB)
    task.set_editor_property("destination_path", "/Game/Characters/")
    task.set_editor_property("destination_name", "SOLARA_CATHEDRAL_AAA")
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    
    # Set up GLTF import options
    options = unreal.GLTFImportOptions()
    options.set_editor_property("import_materials", True)
    options.set_editor_property("import_textures", True)
    options.set_editor_property("import_animations", False)
    
    import_tasks.append(task)
    
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    asset_tools.import_asset_tasks(import_tasks)
    log("  + AssetImportTask submitted with GLTF options")
except Exception as e:
    log(f"  ! AssetImportTask failed: {e}", "WARN")

# Wait for the import
import time
log("Waiting 15s for import to complete...")
time.sleep(15)

# Method 3: Check if it appeared
log("Checking if cathedral asset appeared...")
cathedral_path = "/Game/Characters/SOLARA_CATHEDRAL_AAA"
asset = EAL.load_asset(cathedral_path)
if not asset:
    # Try searching
    assets = EAL.list_assets("/Game/Characters/", recursive=True, include_folder=False)
    cathedral_assets = [a for a in assets if "CATHEDRAL" in a.upper() or "cathedral" in a.lower()]
    log(f"  Found {len(cathedral_assets)} cathedral-related assets:")
    for a in cathedral_assets[:10]:
        log(f"    - {a}")
        loaded = EAL.load_asset(a)
        if loaded:
            asset = loaded
            cathedral_path = a
            break

if asset:
    log(f"  + Cathedral asset loaded: {cathedral_path}")
    log(f"  + Type: {asset.get_class().get_name()}")
    
    # Spawn it in the level
    log("  Spawning cathedral in level...")
    spawned = False
    cls_name = asset.get_class().get_name()
    
    if "StaticMesh" in cls_name:
        actor = ELL.spawn_actor_from_class(unreal.StaticMeshActor, Vec(0, 0, 0), Rot(0, 0, 0))
        if actor:
            actor.set_actor_label("Solara_Cathedral_AAA")
            comp = actor.get_editor_property("static_mesh_component")
            if comp:
                comp.set_static_mesh(asset)
                comp.set_editor_property("visibility", True)
                comp.set_editor_property("cast_shadow", True)
            log("  + Cathedral spawned!")
            spawned = True
    elif "SkeletalMesh" in cls_name:
        log("  ! Cathedral is a SkeletalMesh, not StaticMesh", "WARN")
    
    if not spawned:
        log("  ! Could not spawn cathedral", "WARN")
else:
    log("  ! Cathedral asset not found after import", "WARN")
    log("  The GLB may need manual import via the Content Browser", "WARN")
    log("  File: " + CATHEDRAL_GLB, "WARN")

# Take screenshot
log("Taking screenshot...")
try:
    unreal.AutomationLibrary.take_high_res_screenshot(
        1920, 1080,
        "C:/Users/Tap/Desktop/Stage1_Cathedral_AAA_v2.png"
    )
    log("  + Screenshot saved!")
except Exception as e:
    log(f"  ! Screenshot failed: {e}", "ERROR")

log("=" * 60)
log("CATHEDRAL IMPORT v2 COMPLETE")
log("=" * 60)
