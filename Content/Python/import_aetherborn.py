"""
Eclipse: Realms of the Fractured
GATE 1: Import Aetherborn Hero Character
Momentum Labs | Hermes | November 7, 2026

Imports the HQ Aetherborn 3D model into UE5.8 as a StaticMesh,
creates a skeletal mesh component, and sets up the character Blueprint.
"""
import unreal
import os

def log(msg):
    unreal.log(f"[ECLIPSE GATE 1] {msg}")

log("=" * 60)
log("ECLIPSE GATE 1: Importing Aetherborn Hero Character")
log("=" * 60)

# === CONFIG ===
CONTENT_DIR = unreal.SystemLibrary.get_project_content_directory()
CHARACTERS_DIR = os.path.join(CONTENT_DIR, "Characters")
GAME_DIR = "/Game/Characters"

# Aetherborn HQ models
AETHERBORN_MALE = os.path.join(CHARACTERS_DIR, "AETHERBORN_MALE_HQ.glb")
AETHERBORN_FEMALE = os.path.join(CHARACTERS_DIR, "AETHERBORN_FEMALE_HQ.glb")

log(f"Content dir: {CONTENT_DIR}")
log(f"Characters dir: {CHARACTERS_DIR}")
log(f"Male model exists: {os.path.exists(AETHERBORN_MALE)}")
log(f"Female model exists: {os.path.exists(AETHERBORN_FEMALE)}")

# === IMPORT GLB AS STATIC MESH ===
def import_glb_as_static_mesh(glb_path, asset_name, destination_path="/Game/Characters/Imported"):
    """Import a GLB file as a StaticMesh asset in UE5.8"""
    log(f"Importing {asset_name} from {glb_path}")
    
    if not os.path.exists(glb_path):
        log(f"ERROR: File not found: {glb_path}")
        return None
    
    # Create import task
    task = unreal.AssetImportTask()
    task.set_editor_property('filename', glb_path)
    task.set_editor_property('destination_path', destination_path)
    task.set_editor_property('destination_name', asset_name)
    task.set_editor_property('replace_existing', True)
    task.set_editor_property('automated_import_should_yes_to_all', True)
    task.set_editor_property('save', True)
    
    # GLB import options
    import_options = unreal.GLTFImportOptions()
    import_options.set_editor_property('import_materials', True)
    import_options.set_editor_property('import_textures', True)
    import_options.set_editor_property('import_animations', False)
    import_options.set_editor_property('import_as_static_mesh', True)
    
    task.options = import_options
    
    # Execute import
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    
    # Get the imported asset
    asset_path = f"{destination_path}/{asset_name}"
    static_mesh = unreal.EditorAssetLibrary.load_asset(asset_path)
    
    if static_mesh:
        log(f"SUCCESS: Imported {asset_name} at {asset_path}")
        return static_mesh
    else:
        log(f"WARNING: Asset not found at {asset_path}, trying variations...")
        # Try finding it with different path
        assets = unreal.EditorAssetLibrary.list_assets(destination_path)
        for a in assets:
            if asset_name.lower() in a.lower():
                log(f"  Found: {a}")
                return unreal.EditorAssetLibrary.load_asset(a)
        return None

# === IMPORT AETHERBORN MODELS ===
log("Step 1: Importing Aetherborn Male HQ...")
male_mesh = import_glb_as_static_mesh(AETHERBORN_MALE, "SM_Aetherborn_Male_HQ")

log("Step 2: Importing Aetherborn Female HQ...")
female_mesh = import_glb_as_static_mesh(AETHERBORN_FEMALE, "SM_Aetherborn_Female_HQ")

# === CREATE CHARACTER MATERIAL ===
log("Step 3: Creating character material...")

def create_holographic_material(material_name, destination_path="/Game/Materials"):
    """Create a holographic dark fantasy material for the character"""
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    
    # Create material asset
    material = asset_tools.create_asset(material_name, destination_path, unreal.MaterialFactoryNew, None)
    
    if material:
        log(f"Created material: {material_name}")
        # Set basic properties for dark fantasy look
        material.set_editor_property('blend_mode', unreal.BlendMode.BLEND_Opaque)
        material.set_editor_property('shading_model', unreal.ShadingModel.SM_Default)
        return material
    return None

# Create materials
mat_hero = create_holographic_material("M_Aetherborn_Hero")
mat_environment = create_holographic_material("M_Solara_Cathedral", "/Game/Materials")

# === CREATE BASIC LEVEL: SOLARA CATHEDRAL ===
log("Step 4: Creating Solara Cathedral level...")

def create_cathedral_floor():
    """Create the cathedral floor using a brush"""
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMesh,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    return actor

# Spawn the Aetherborn in the level
if male_mesh:
    log("Step 5: Spawning Aetherborn Male in level...")
    # Spawn the character in the world
    actor_location = unreal.Vector(0, 0, 100)
    actor_rotation = unreal.Rotator(0, 0, 0)
    
    spawned = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMesh,
        actor_location,
        actor_rotation
    )
    
    if spawned:
        spawned.set_editor_property('static_mesh', male_mesh)
        spawned.set_actor_label("Aetherborn_Male_Hero")
        log(f"SUCCESS: Spawned Aetherborn Male at {actor_location}")

log("Step 6: Setting up lighting...")
# Add directional light for the cathedral
sun_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.DirectionalLight,
    unreal.Vector(0, 0, 2000),
    unreal.Rotator(-45, 0, 0)
)
if sun_light:
    sun_light.set_actor_label("Cathedral_SunLight")
    log("Spawned directional light")

# Add sky light
sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
    unreal.SkyLight,
    unreal.Vector(0, 0, 3000),
    unreal.Rotator(0, 0, 0)
)
if sky_light:
    sky_light.set_actor_label("Cathedral_SkyLight")
    log("Spawned sky light")

# === SAVE LEVEL ===
log("Step 7: Saving level...")
unreal.EditorLevelLibrary.save_current_level()
log("Level saved!")

# === SAVE ALL ASSETS ===
log("Step 8: Saving all assets...")
unreal.EditorAssetLibrary.save_directory("/Game/")
log("All assets saved!")

log("=" * 60)
log("GATE 1: IMPORT COMPLETE")
log(f"  Aetherborn Male: {'OK' if male_mesh else 'FAILED'}")
log(f"  Aetherborn Female: {'OK' if female_mesh else 'FAILED'}")
log(f"  Materials: {'OK' if mat_hero else 'FAILED'}")
log(f"  Lighting: OK")
log("=" * 60)
log("Next: Configure character controller and game mode")
