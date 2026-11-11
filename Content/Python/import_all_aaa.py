"""
Import all 13 AAA ancestry contact sheet models into UE5.8.
Each GLB imports as StaticMesh + Material + Textures.
Run inside UE5.8:  py C:/Users/Tap/EclipseRotF/Content/Python/import_all_aaa.py
"""
import os, time, json
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "import_all_aaa.log")
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
    log("ERROR: unreal not available!")
    raise

log("=" * 60)
log("ECLIPSE — Import All 13 AAA Ancestry Models")
log("=" * 60)

EAL = unreal.EditorAssetLibrary
ELL = unreal.EditorLevelLibrary

AAA_DIR = r"K:\Eclipse_Data\AAA_GLBs"
DEST_PATH = "/Game/Characters/AAA_ContactSheet/"

# Ensure destination folder exists
try:
    EAL.make_directory(DEST_PATH)
except:
    pass

# All 13 ancestry GLB files
models = [
    "AETHERBORN_AAA_ContactSheet.glb",
    "SOLARI_AAA_ContactSheet.glb",
    "VYRAEL_AAA_ContactSheet.glb",
    "THARUK_AAA_ContactSheet.glb",
    "KAZARI_AAA_ContactSheet.glb",
    "UMBRALIS_AAA_ContactSheet.glb",
    "DRAVEN_AAA_ContactSheet.glb",
    "GRIMSTARR_AAA_ContactSheet.glb",
    "NEREYTH_AAA_ContactSheet.glb",
    "MYCELIAR_AAA_ContactSheet.glb",
    "FORGEBORN_AAA_ContactSheet.glb",
    "HELLSWORN_AAA_ContactSheet.glb",
    "NECREVAN_AAA_ContactSheet.glb",
]

imported = 0
failed = 0

for i, glb_file in enumerate(models, 1):
    glb_path = os.path.join(AAA_DIR, glb_file)
    ancestry_name = glb_file.replace("_AAA_ContactSheet.glb", "")
    
    if not os.path.exists(glb_path):
        log(f"  [{i}/13] ! {glb_file} — NOT FOUND on disk")
        failed += 1
        continue
    
    size_mb = os.path.getsize(glb_path) / (1024 * 1024)
    log(f"  [{i}/13] Importing {ancestry_name} ({size_mb:.1f}MB)...")
    
    try:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", glb_path)
        task.set_editor_property("destination_path", DEST_PATH)
        task.set_editor_property("destination_name", ancestry_name)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("automated", True)
        task.set_editor_property("save", True)
        
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        asset_tools.import_asset_tasks([task])
        
        # Wait for import to complete
        time.sleep(5)
        
        # Verify it imported
        mesh_path = f"{DEST_PATH}{ancestry_name}/StaticMeshes/{ancestry_name}.{ancestry_name}"
        mesh = EAL.load_asset(mesh_path)
        
        if mesh:
            log(f"    + {ancestry_name} imported OK")
            imported += 1
        else:
            # Try alternate path
            assets = EAL.list_assets(f"{DEST_PATH}{ancestry_name}/", recursive=True, include_folder=False)
            if assets:
                log(f"    + {ancestry_name} imported ({len(assets)} assets)")
                for a in assets[:3]:
                    log(f"      - {a}")
                imported += 1
            else:
                log(f"    ! {ancestry_name} import may have failed (no assets found)")
                failed += 1
                
    except Exception as e:
        log(f"    ! {ancestry_name} failed: {e}", "ERROR")
        failed += 1

log(f"---")
log(f"RESULTS: {imported}/13 imported, {failed} failed")

# List all imported assets
log("---")
log("All AAA ContactSheet assets in Content Browser:")
all_assets = EAL.list_assets(DEST_PATH, recursive=True, include_folder=False)
log(f"Total assets: {len(all_assets)}")
for a in all_assets:
    loaded = EAL.load_asset(a)
    if loaded and "StaticMesh" in loaded.get_class().get_name():
        log(f"  MESH: {a}")

log("=" * 60)
log("IMPORT ALL AAA COMPLETE")
log("=" * 60)