"""
Eclipse: Render ALL character models from GLB with Cycles (textured)
Cycles renderer reads PBR textures properly
"""
import bpy
import os
import math
import glob

OUTPUT_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters\Blender_Output\Renders"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def log(msg):
    print(f"[RENDER] {msg}", flush=True)

def render_glb_cycles(glb_path, output_name):
    """Render GLB with Cycles for proper PBR texture display"""
    # Clear scene
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for coll in [bpy.data.meshes, bpy.data.materials, bpy.data.lights, bpy.data.cameras, bpy.data.images]:
        for item in list(coll):
            if item.users == 0:
                coll.remove(item)

    # Import GLB
    try:
        bpy.ops.import_scene.gltf(filepath=glb_path)
    except Exception as e:
        log(f"  Import failed: {e}")
        return False

    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    if not meshes:
        return False

    obj = meshes[0]
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')
    obj.location = (0, 0, 0)
    bpy.ops.object.shade_smooth()

    # 3-point lighting
    key = bpy.data.lights.new(name="Key", type='AREA')
    key.energy = 100000
    key.size = 5
    key.color = (1.0, 0.95, 0.9)
    key_obj = bpy.data.objects.new("Key", key)
    bpy.context.collection.objects.link(key_obj)
    key_obj.location = (4, -4, 5)
    key_obj.rotation_euler = (math.radians(50), 0, math.radians(45))

    fill = bpy.data.lights.new(name="Fill", type='AREA')
    fill.energy = 40000
    fill.size = 5
    fill.color = (0.8, 0.85, 1.0)
    fill_obj = bpy.data.objects.new("Fill", fill)
    bpy.context.collection.objects.link(fill_obj)
    fill_obj.location = (-4, -3, 3)
    fill_obj.rotation_euler = (math.radians(55), 0, math.radians(-35))

    rim = bpy.data.lights.new(name="Rim", type='AREA')
    rim.energy = 60000
    rim.size = 5
    rim.color = (1.0, 0.8, 0.6)
    rim_obj = bpy.data.objects.new("Rim", rim)
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
        bg.inputs[0].default_value = (0.02, 0.02, 0.04, 1.0)
        bg.inputs[1].default_value = 1.0

    # Camera
    cam_data = bpy.data.cameras.new("Cam")
    cam = bpy.data.objects.new("Camera", cam_data)
    bpy.context.collection.objects.link(cam)
    cam.location = (5, -5, 2.5)
    cam.rotation_euler = (math.radians(72), 0, math.radians(45))
    bpy.context.scene.camera = cam

    # Render with Cycles for proper PBR
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.samples = 64  # Fast but enough for texture preview
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 768
    scene.render.resolution_y = 768
    scene.render.image_settings.file_format = 'PNG'

    # 3/4 view
    scene.render.filepath = os.path.join(OUTPUT_DIR, f"{output_name}_3quarter.png")
    log(f"  Rendering {output_name} 3/4 (Cycles)...")
    bpy.ops.render.render(write_still=True)

    # Front view
    cam.location = (0, -7, 2)
    cam.rotation_euler = (math.radians(78), 0, 0)
    scene.render.filepath = os.path.join(OUTPUT_DIR, f"{output_name}_front.png")
    log(f"  Rendering {output_name} front (Cycles)...")
    bpy.ops.render.render(write_still=True)

    return True

def main():
    log("=" * 50)
    log("ECLIPSE RENDERS — CYCLES (PBR TEXTURES)")
    log("=" * 50)

    input_dir = r"C:\Users\Tap\EclipseRotF\Content\Characters"
    glb_files = sorted(glob.glob(os.path.join(input_dir, "*_HQ.glb")))

    # Priority: hero characters first
    priority = ["AETHERBORN_MALE", "AETHERBORN_FEMALE", "SOLARI", "THARUK",
                "NEREYTH", "KAZARI", "DRAVEN", "TEMPLAR", "ARCANIST", "BERSERKER"]

    def priority_key(filepath):
        name = os.path.basename(filepath)
        for i, p in enumerate(priority):
            if p in name:
                return i
        return len(priority)

    glb_files.sort(key=priority_key)

    log(f"Rendering {len(glb_files)} models with Cycles...")

    for i, glb in enumerate(glb_files):
        name = os.path.splitext(os.path.basename(glb))[0]
        log(f"\n[{i+1}/{len(glb_files)}] {name}")
        try:
            render_glb_cycles(glb, name)
        except Exception as e:
            log(f"  Error: {e}")

    log(f"\nAll renders done! In: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
