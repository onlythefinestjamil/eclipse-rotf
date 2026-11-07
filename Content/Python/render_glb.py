"""
Eclipse: Render character models FROM GLB (with textures)
Renders the original Meshy GLB files which have full PBR materials
"""
import bpy
import os
import math
import glob

OUTPUT_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters\Blender_Output\Renders"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def log(msg):
    print(f"[RENDER] {msg}", flush=True)

def render_glb(glb_path, output_name):
    """Render a GLB file with full PBR materials"""
    # Clear scene
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for coll in [bpy.data.meshes, bpy.data.materials, bpy.data.lights, bpy.data.cameras, bpy.data.images]:
        for item in list(coll):
            if item.users == 0:
                coll.remove(item)

    # Import GLB (preserves PBR materials + textures)
    try:
        bpy.ops.import_scene.gltf(filepath=glb_path)
    except Exception as e:
        log(f"  Import failed: {e}")
        return False

    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    if not meshes:
        log(f"  No mesh found")
        return False

    obj = meshes[0]
    
    # Center
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')
    obj.location = (0, 0, 0)
    
    # Shade smooth
    bpy.ops.object.shade_smooth()

    # 3-point lighting
    key = bpy.data.lights.new(name="Key", type='AREA')
    key.energy = 80000
    key.size = 5
    key_obj = bpy.data.objects.new("KeyLight", key)
    bpy.context.collection.objects.link(key_obj)
    key_obj.location = (4, -4, 5)
    key_obj.rotation_euler = (math.radians(50), 0, math.radians(45))

    fill = bpy.data.lights.new(name="Fill", type='AREA')
    fill.energy = 30000
    fill.size = 5
    fill_obj = bpy.data.objects.new("FillLight", fill)
    bpy.context.collection.objects.link(fill_obj)
    fill_obj.location = (-4, -3, 3)
    fill_obj.rotation_euler = (math.radians(55), 0, math.radians(-35))

    rim = bpy.data.lights.new(name="Rim", type='AREA')
    rim.energy = 50000
    rim.size = 5
    rim_obj = bpy.data.objects.new("RimLight", rim)
    bpy.context.collection.objects.link(rim_obj)
    rim_obj.location = (0, 6, 4)
    rim_obj.rotation_euler = (math.radians(130), 0, 0)

    # Dark background
    world = bpy.context.scene.world
    if not world:
        world = bpy.data.worlds.new("World")
        bpy.context.scene.world = world
    world.use_nodes = True
    bg = world.node_tree.nodes.get("Background")
    if bg:
        bg.inputs[0].default_value = (0.03, 0.03, 0.05, 1.0)
        bg.inputs[1].default_value = 1.0

    # Camera — 3/4 hero view
    cam_data = bpy.data.cameras.new("Camera")
    cam = bpy.data.objects.new("Camera", cam_data)
    bpy.context.collection.objects.link(cam)
    cam.location = (5, -5, 2.5)
    cam.rotation_euler = (math.radians(72), 0, math.radians(45))
    bpy.context.scene.camera = cam

    # Render settings
    scene = bpy.context.scene
    try:
        scene.render.engine = 'BLENDER_EEVEE_NEXT'
    except:
        scene.render.engine = 'BLENDER_EEVEE'
    scene.render.resolution_x = 1024
    scene.render.resolution_y = 1024
    scene.render.image_settings.file_format = 'PNG'
    scene.render.filepath = os.path.join(OUTPUT_DIR, f"{output_name}_3quarter.png")

    log(f"  Rendering {output_name} 3/4...")
    bpy.ops.render.render(write_still=True)

    # Front view
    cam.location = (0, -7, 2)
    cam.rotation_euler = (math.radians(78), 0, 0)
    scene.render.filepath = os.path.join(OUTPUT_DIR, f"{output_name}_front.png")
    log(f"  Rendering {output_name} front...")
    bpy.ops.render.render(write_still=True)

    return True

def main():
    log("=" * 50)
    log("ECLIPSE CHARACTER RENDERS — FROM GLB (PBR)")
    log("=" * 50)

    input_dir = r"C:\Users\Tap\EclipseRotF\Content\Characters"
    glb_files = sorted(glob.glob(os.path.join(input_dir, "*_HQ.glb")))

    # Priority order
    priority = ["AETHERBORN_MALE", "AETHERBORN_FEMALE", "SOLARI", "THARUK",
                "NEREYTH", "KAZARI", "DRAVEN", "TEMPLAR", "ARCANIST", "BERSERKER"]
    
    def priority_key(filepath):
        name = os.path.basename(filepath)
        for i, p in enumerate(priority):
            if p in name:
                return i
        return len(priority)

    glb_files.sort(key=priority_key)
    
    # Render first 5 (most important)
    for i, glb in enumerate(glb_files[:5]):
        name = os.path.splitext(os.path.basename(glb))[0]
        log(f"\n[{i+1}] {name}")
        try:
            render_glb(glb, name)
        except Exception as e:
            log(f"  Error: {e}")

    log(f"\nDone! Renders in: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
