"""
Eclipse Stage 1 — Place cathedral as environment + fix orange lighting.
The cathedral StaticMesh is already imported at:
/Game/Characters/SOLARA_CATHEDRAL_AAA/StaticMeshes/SOLARA_CATHEDRAL_AAA.SOLARA_CATHEDRAL_AAA
Run inside UE5.8:  py C:/Users/Tap/EclipseRotF/Content/Python/stage1_cathedral_environment.py
"""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_cathedral_environment.log")

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
log("ECLIPSE STAGE 1 — Cathedral as Environment + Fix Lighting")
log("=" * 60)

ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary
Vec = unreal.Vector
Rot = unreal.Rotator

# 1. Load the cathedral StaticMesh (not the material)
CATHEDRAL_MESH_PATH = "/Game/Characters/SOLARA_CATHEDRAL_AAA/StaticMeshes/SOLARA_CATHEDRAL_AAA.SOLARA_CATHEDRAL_AAA"
log(f"1. Loading cathedral StaticMesh from {CATHEDRAL_MESH_PATH}...")
cathedral_mesh = EAL.load_asset(CATHEDRAL_MESH_PATH)
if cathedral_mesh:
    log(f"  + Cathedral StaticMesh loaded: {cathedral_mesh.get_name()}")
    log(f"  + Type: {cathedral_mesh.get_class().get_name()}")
else:
    log("  ! Could not load cathedral mesh at that path", "ERROR")
    # Try searching for it
    assets = EAL.list_assets("/Game/Characters/SOLARA_CATHEDRAL_AAA/", recursive=True, include_folder=False)
    log(f"  Searching... found {len(assets)} assets:")
    for a in assets:
        log(f"    - {a}")
        loaded = EAL.load_asset(a)
        if loaded and "StaticMesh" in loaded.get_class().get_name():
            cathedral_mesh = loaded
            CATHEDRAL_MESH_PATH = a
            log(f"  + Found StaticMesh at: {a}")
            break

# 2. Spawn the cathedral as environment geometry
log("2. Spawning cathedral as environment...")
if cathedral_mesh:
    actor = ELL.spawn_actor_from_class(
        unreal.StaticMeshActor,
        Vec(0, 0, 0),
        Rot(0, 0, 0),
    )
    if actor:
        actor.set_actor_label("Solara_Cathedral_Environment")
        comp = actor.get_editor_property("static_mesh_component")
        if comp:
            comp.set_static_mesh(cathedral_mesh)
        try:
            comp.set_editor_property("cast_shadow", True)
        except:
            pass
        # Make it large — this is the environment, not a prop
        actor.set_actor_scale3d(Vec(10, 10, 10))
        log("  + Cathedral environment spawned at origin (scale 10x)")
else:
    log("  ! No cathedral mesh available", "ERROR")

# 3. Fix the orange lighting — reduce intensity, cooler color
log("3. Fixing orange lighting...")
for a in ELL.get_all_level_actors():
    try:
        cls = a.get_class().get_name()
        label = a.get_actor_label()
        if cls == "DirectionalLight":
            comp = a.get_editor_property("light_component")
            if comp:
                comp.set_editor_property("intensity", 5.0)  # Much lower
                # Set to neutral white, not warm orange
                try:
                    comp.set_editor_property("light_color", (255, 250, 245, 255))
                except:
                    pass
                log(f"  + {label}: intensity=5.0, neutral white")
        elif cls == "SkyLight":
            comp = a.get_editor_property("light_component")
            if comp:
                comp.set_editor_property("intensity", 2.0)
                log(f"  + {label}: intensity=2.0")
        elif cls == "ExponentialHeightFog":
            comp = a.get_editor_property("component")
            if comp:
                comp.set_editor_property("fog_density", 0.008)
                comp.set_editor_property("fog_max_opacity", 0.35)
                log(f"  + {label}: density=0.008, opacity=0.35")
    except:
        pass

# 4. Reposition characters inside the cathedral
log("4. Repositioning characters inside cathedral...")
for a in ELL.get_all_level_actors():
    try:
        label = a.get_actor_label()
        if "Aetherborn" in label or "Stage1_Player" in label:
            a.set_actor_location(Vec(0, 0, 100))
            a.set_actor_rotation(Rot(0, 0, 0))
            log(f"  + {label} at (0, 0, 100)")
        elif "Berserker" in label or "Stage1_Enemy" in label:
            a.set_actor_location(Vec(500, 500, 100))
            a.set_actor_rotation(Rot(0, 180, 0))
            log(f"  + {label} at (500, 500, 100)")
    except:
        pass

# 5. List all actors
log("5. All actors:")
for a in ELL.get_all_level_actors():
    try:
        loc = a.get_actor_location()
        log(f"  - {a.get_actor_label()} ({a.get_class().get_name()}) @ ({int(loc.x)}, {int(loc.y)}, {int(loc.z)})")
    except:
        pass

# 6. Take screenshot
log("6. Taking screenshot...")
try:
    unreal.AutomationLibrary.take_high_res_screenshot(
        1920, 1080,
        "C:/Users/Tap/Desktop/Stage1_Cathedral_Environment.png"
    )
    log("  + Screenshot saved!")
except Exception as e:
    log(f"  ! Screenshot failed: {e}", "ERROR")

log("=" * 60)
log("CATHEDRAL ENVIRONMENT COMPLETE")
log("=" * 60)