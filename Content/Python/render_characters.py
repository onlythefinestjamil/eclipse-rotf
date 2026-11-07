"""
Eclipse: Render character model screenshots
Renders front + 3/4 view of each Blender-processed model
"""
import bpy
import os
import math

OUTPUT_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters\Blender_Output\Renders"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def log(msg):
    print(f"[RENDER] {msg}", flush=True)

def render_model(obj_path, output_name):
    """Render a model from front and 3/4 angle"""
    # Clear scene
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for coll in [bpy.data.meshes, bpy.data.materials, bpy.data.lights, bpy.data.cameras]:
        for item in list(coll):
            if item.users == 0:
                coll.remove(item)

    # Import OBJ
    try:
        bpy.ops.wm.obj_import(filepath=obj_path)
    except:
        try:
            bpy.ops.import_scene.obj(filepath=obj_path)
        except Exception as e:
            log(f"  Import failed: {e}")
            return False

    # Get the mesh
    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    if not meshes:
        log(f"  No mesh found")
        return False

    obj = meshes[0]

    # Center the object
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')
    obj.location = (0, 0, 0)

    # Shade smooth
    bpy.ops.object.shade_smooth()

    # Add lighting
    # Key light
    key_light = bpy.data.lights.new(name="Key", type='AREA')
    key_light.energy = 50000
    key_light.size = 5
    key_obj = bpy.data.objects.new("KeyLight", key_light)
    bpy.context.collection.objects.link(key_obj)
    key_obj.location = (3, -3, 4)
    key_obj.rotation_euler = (math.radians(45), 0, math.radians(45))

    # Fill light
    fill_light = bpy.data.lights.new(name="Fill", type='AREA')
    fill_light.energy = 20000
    fill_light.size = 5
    fill_obj = bpy.data.objects.new("FillLight", fill_light)
    bpy.context.collection.objects.link(fill_obj)
    fill_obj.location = (-3, -2, 3)
    fill_obj.rotation_euler = (math.radians(50), 0, math.radians(-30))

    # Rim light
    rim_light = bpy.data.lights.new(name="Rim", type='AREA')
    rim_light.energy = 30000
    rim_light.size = 5
    rim_obj = bpy.data.objects.new("RimLight", rim_light)
    bpy.context.collection.objects.link(rim_obj)
    rim_obj.location = (0, 5, 3)
    rim_obj.rotation_euler = (math.radians(120), 0, 0)

    # World background — dark
    world = bpy.context.scene.world
    if not world:
        world = bpy.data.worlds.new("World")
        bpy.context.scene.world = world
    world.use_nodes = True
    bg = world.node_tree.nodes.get("Background")
    if bg:
        bg.inputs[0].default_value = (0.02, 0.02, 0.03, 1.0)
        bg.inputs[1].default_value = 1.0

    # Camera — 3/4 view
    cam_data = bpy.data.cameras.new("Camera")
    cam = bpy.data.objects.new("Camera", cam_data)
    bpy.context.collection.objects.link(cam)
    cam.location = (4, -4, 2.5)
    cam.rotation_euler = (math.radians(75), 0, math.radians(45))
    bpy.context.scene.camera = cam

    # Render settings
    scene = bpy.context.scene
    scene.render.engine = 'BLENDER_EEVEE_NEXT' if 'BLENDER_EEVEE_NEXT' in dir(bpy.types) else 'BLENDER_EEVEE'
    scene.render.resolution_x = 1024
    scene.render.resolution_y = 1024
    scene.render.image_settings.file_format = 'PNG'
    scene.render.filepath = os.path.join(OUTPUT_DIR, f"{output_name}_3quarter.png")

    # Render 3/4 view
    log(f"  Rendering {output_name} 3/4 view...")
    bpy.ops.render.render(write_still=True)

    # Front view
    cam.location = (0, -6, 2)
    cam.rotation_euler = (math.radians(80), 0, 0)
    scene.render.filepath = os.path.join(OUTPUT_DIR, f"{output_name}_front.png")
    log(f"  Rendering {output_name} front view...")
    bpy.ops.render.render(write_still=True)

    return True

def main():
    log("=" * 50)
    log("ECLIPSE CHARACTER RENDERS")
    log("=" * 50)

    input_dir = r"C:\Users\Tap\EclipseRotF\Content\Characters\Blender_Output"

    import glob
    obj_files = sorted(glob.glob(os.path.join(input_dir, "*_blender.obj")))

    # Render the most important ones first
    priority = ["AETHERBORN_MALE", "AETHERBORN_FEMALE", "SOLARI", "THARUK",
                "NEREYTH", "KAZARI", "DRAVEN", "TEMPLAR", "ARCANIST", "BERSERKER"]

    # Sort by priority
    def priority_key(filepath):
        name = os.path.basename(filepath)
        for i, p in enumerate(priority):
            if p in name:
                return i
        return len(priority)

    obj_files.sort(key=priority_key)

    log(f"Found {len(obj_files)} models to render")
    log(f"Rendering priority characters first...")

    for i, obj in enumerate(obj_files[:10]):  # Render first 10
        name = os.path.splitext(os.path.basename(obj))[0].replace("_blender", "")
        log(f"\n[{i+1}] {name}")
        try:
            render_model(obj, name)
        except Exception as e:
            log(f"  Error: {e}")

    log(f"\nRenders saved to: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
