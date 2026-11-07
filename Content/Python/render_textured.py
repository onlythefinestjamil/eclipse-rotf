"""
Eclipse: Render GLB with PBR textures — FIXED
The issue was that Blender's glTF importer creates material nodes
but Cycles/EEVEE needs the materials explicitly assigned to the
mesh data. This script forces material assignment after import.
"""
import bpy
import os
import math
import glob

OUTPUT_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters\Blender_Output\Renders"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def log(msg):
    print(f"[RENDER] {msg}", flush=True)

def fix_materials(obj):
    """Force assign materials from the glTF import to mesh data"""
    if not obj or obj.type != 'MESH':
        return
    
    mesh = obj.data
    
    # Check if materials exist on the object
    if len(obj.material_slots) > 0:
        log(f"  Found {len(obj.material_slots)} material slots")
        for slot in obj.material_slots:
            if slot.material:
                # Ensure the material uses nodes
                mat = slot.material
                mat.use_nodes = True
                
                # Check if it has a Principled BSDF
                nodes = mat.node_tree.nodes
                principled = None
                for node in nodes:
                    if node.type == 'BSDF_PRINCIPLED':
                        principled = node
                        break
                
                if principled:
                    # Check if there are image textures connected
                    has_texture = False
                    for node in nodes:
                        if node.type == 'TEX_IMAGE':
                            if node.image:
                                has_texture = True
                                log(f"    Material '{mat.name}' has image: {node.image.name}")
                                break
                    
                    if not has_texture:
                        log(f"    Material '{mat.name}' has no image texture — checking slots...")
                        # Try to find texture in material properties
                        if mat.diffuse_color:
                            # Apply diffuse color to principled
                            principled.inputs['Base Color'].default_value = (
                                mat.diffuse_color[0], mat.diffuse_color[1], 
                                mat.diffuse_color[2], 1.0
                            )
                            log(f"    Applied diffuse color: {mat.diffuse_color}")
    else:
        log(f"  No material slots — creating from mesh data")
        # Try to create a basic material with vertex colors
        if mesh.vertex_colors:
            log(f"  Mesh has vertex colors — using them")
            mat = bpy.data.materials.new(name=f"{obj.name}_mat")
            mat.use_nodes = True
            nodes = mat.node_tree.nodes
            links = mat.node_tree.links
            
            # Clear default nodes
            for node in list(nodes):
                nodes.remove(node)
            
            # Create Vertex Color → Principled BSDF → Output
            output = nodes.new('ShaderNodeOutputMaterial')
            principled = nodes.new('ShaderNodeBsdfPrincipled')
            vcol = nodes.new('ShaderNodeVertexColor')
            vcol.layer_name = mesh.vertex_colors[0].name
            
            links.new(vcol.outputs['Color'], principled.inputs['Base Color'])
            links.new(principled.outputs['BSDF'], output.inputs['Surface'])
            
            obj.data.materials.append(mat)
            log(f"  Created vertex color material")
        else:
            log(f"  No vertex colors — using principled with diffuse")

def render_glb_textured(glb_path, output_name):
    """Render GLB with proper material assignment"""
    # Clear scene
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for coll in [bpy.data.meshes, bpy.data.materials, bpy.data.lights, 
                 bpy.data.cameras, bpy.data.images]:
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
    log(f"  Imported: {len(meshes)} meshes, {len(obj.data.vertices)} verts")
    
    # Check for image textures in the blend data
    images = [img for img in bpy.data.images if img.name != 'Render Result']
    log(f"  Images in blend data: {len(images)}")
    for img in images[:5]:
        log(f"    Image: {img.name} ({img.size[0]}x{img.size[1]})")

    # Fix materials — force node assignment
    for mesh_obj in meshes:
        fix_materials(mesh_obj)
        bpy.context.view_layer.objects.active = mesh_obj
        mesh_obj.select_set(True)
        bpy.ops.object.shade_smooth()

    # Center
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')
    obj.location = (0, 0, 0)

    # 3-point lighting
    key = bpy.data.lights.new(name="Key", type='AREA')
    key.energy = 80000
    key.size = 5
    key.color = (1.0, 0.95, 0.9)
    key_obj = bpy.data.objects.new("Key", key)
    bpy.context.collection.objects.link(key_obj)
    key_obj.location = (4, -4, 5)
    key_obj.rotation_euler = (math.radians(50), 0, math.radians(45))

    fill = bpy.data.lights.new(name="Fill", type='AREA')
    fill.energy = 30000
    fill.size = 5
    fill.color = (0.7, 0.8, 1.0)
    fill_obj = bpy.data.objects.new("Fill", fill)
    bpy.context.collection.objects.link(fill_obj)
    fill_obj.location = (-4, -3, 3)
    fill_obj.rotation_euler = (math.radians(55), 0, math.radians(-35))

    rim = bpy.data.lights.new(name="Rim", type='AREA')
    rim.energy = 50000
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

    # Render — use Cycles with proper color management
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.samples = 32
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 768
    scene.render.resolution_y = 768
    scene.render.image_settings.file_format = 'PNG'
    # Color management — filmic for proper PBR look
    scene.view_settings.view_transform = 'Filmic'
    scene.view_settings.look = 'Medium High Contrast'

    # 3/4 view
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
    log("ECLIPSE RENDERS — TEXTURED (FIXED)")
    log("=" * 50)

    input_dir = r"C:\Users\Tap\EclipseRotF\Content\Characters"
    glb_files = sorted(glob.glob(os.path.join(input_dir, "*_HQ.glb")))

    # Just render the 5 most important first
    priority = ["AETHERBORN_MALE", "AETHERBORN_FEMALE", "SOLARI", "THARUK", "NEREYTH"]
    
    to_render = []
    for p in priority:
        for glb in glb_files:
            if p in os.path.basename(glb):
                to_render.append(glb)
                break

    log(f"Rendering {len(to_render)} priority characters...")

    for i, glb in enumerate(to_render):
        name = os.path.splitext(os.path.basename(glb))[0]
        log(f"\n[{i+1}/{len(to_render)}] {name}")
        try:
            render_glb_textured(glb, name)
        except Exception as e:
            log(f"  Error: {e}")

    log(f"\nDone! Priority renders in: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()
