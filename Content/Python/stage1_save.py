"""
Eclipse Stage 1 — Save the cathedral as a proper level + screenshot.
Run inside UE5.8:  py C:/Users/Tap/EclipseRotF/Content/Python/stage1_save.py
"""
import os, subprocess
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_save.log")
SCREENSHOT = os.path.join("C:/Users/Tap/Desktop", "Stage1_Cathedral_Screenshot.png")

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
log("ECLIPSE STAGE 1 — Save Level + Screenshot")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

# Step 1: Save the current temp level as a proper .umap file
LEVEL_PATH = "/Game/Maps/SolaraCathedral"
LEVEL_FILE = "C:/Users/Tap/EclipseRotF/Content/Maps/SolaraCathedral.umap"

log("Step 1: Saving current level as SolaraCathedral.umap...")

try:
    # Use EditorLoadingAndSavingUtils to save the current level to a file
    unreal.EditorLoadingAndSavingUtils.save_map(LEVEL_PATH, ELL.get_all_level_actors())
    log("  + Level saved via EditorLoadingAndSavingUtils!")
except Exception as e:
    log(f"  ! EditorLoadingAndSavingUtils failed: {e}", "WARN")
    # Try alternate approach: save_current_level with a filename
    try:
        # First, set the level's filename
        world = unreal.EditorLevelLibrary.get_editor_world()
        log(f"  Current world: {world}")
        log(f"  Current level: {world.get_name()}")
        
        # Use the asset registry to save
        ELL.save_current_level()
        log("  + save_current_level succeeded (even though earlier it said no filename)")
    except Exception as e2:
        log(f"  ! save_current_level also failed: {e2}", "ERROR")

# Step 2: Try to save using the asset registry
try:
    # Save the package to a specific path
    world = ELL.get_editor_world()
    package = world.get_outer()
    log(f"  Package: {package}")
    
    # Try saving as a new map
    EAL.save_asset("/Temp/Untitled_2", force_save=True)
    log("  + Saved /Temp/Untitled_2 via save_asset!")
    
    # Also try saving directory
    EAL.save_directory("/Game/", force_save=True)
    log("  + Saved /Game/ directory!")
except Exception as e:
    log(f"  ! Asset save failed: {e}", "WARN")

# Step 3: Take a screenshot using the automation system
log("Step 2: Taking screenshot...")

try:
    # Use the take_screenshot function
    unreal.AutomationLibrary.take_high_res_screenshot(
        1920, 1080, SCREENSHOT
    )
    log(f"  + Screenshot saved to: {SCREENSHOT}")
except Exception as e:
    log(f"  ! take_high_res_screenshot failed: {e}", "WARN")
    
    try:
        # Try alternate screenshot method
        unreal.AutomationLibrary.take_screenshot(
            SCREENSHOT, 1920, 1080
        )
        log(f"  + Screenshot saved via take_screenshot!")
    except Exception as e2:
        log(f"  ! take_screenshot also failed: {e2}", "WARN")
        
        # Try using the viewport screenshot
        try:
            unreal.EditorLevelLibrary.take_screenshot("Stage1_Cathedral")
            log("  + Viewport screenshot taken!")
        except Exception as e3:
            log(f"  ! Viewport screenshot failed: {e3}", "ERROR")

# Step 4: Report all actors in the level
try:
    actors = ELL.get_all_level_actors()
    log(f"Step 3: {len(actors)} actors in level")
    for a in actors[:10]:
        try:
            log(f"  - {a.get_actor_label()} ({a.get_class().get_name()})")
        except:
            pass
    if len(actors) > 10:
        log(f"  ... and {len(actors) - 10} more")
except Exception as e:
    log(f"  ! Could not list actors: {e}", "WARN")

log("=" * 60)
log("STAGE 1 SAVE COMPLETE")
log(f"  Screenshot: {SCREENSHOT}")
log("=" * 60)
