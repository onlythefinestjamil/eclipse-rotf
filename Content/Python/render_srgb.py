"""
Eclipse: Final render attempt — force sRGB color space on textures
"""
import bpy
import os
import math
import glob

OUTPUT_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters\Blender_Output\Renders"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def log(msg):
    print(f"[RENDER] {msg}", flush=True)

def fix_and_render(glb_path, output_name):
    # Clear
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for coll in [bpy.data.meshes, bpy.data.materials, bpy.data.lights, bpy.data.cameras, bpy.data.images]:
        for item in list(coll):
            if item.users == 0:
                coll.remove(item)

    # Import
    try:
        bpy.ops.import_scene.gltf(filepath=glb_path)
    except Exception as e:
        log(f"  Import failed: {e}")
        return False

    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    if not meshes:
        return False

    obj = meshes[0]
    
    # Log all images and their properties
    for img in bpy.data.images:
        if img.name == 'Render Result':
            continue
        log(f"  Image: {img.name} {img.size[0]}x{img.size[1]} depth={img.depth} colorspace={img.colorspace_settings.name} packed={img.packed_file is not None}")
        # Force sRGB for color textures
        if img.depth in (24, 32, 96, 128):  # Color images
            img.colorspace_settings.name = 'sRGB'
            log(f"    Set to sRGB")
    
    # Rebuild materials from scratch
    for slot in obj.material_slots:
        if not slot.material:
            continue
        mat = slot.material
        mat.use_nodes = True
        nodes = mat.node_tree.nodes
        links = mat.node_tree.links
        
        # Clear all nodes
        for n in list(nodes):
            nodes.remove(n)
        
        # Create output + principled
        output = nodes.new('ShaderNodeOutputMaterial')
        principled = nodes.new('ShaderNodeBsdfPrincipled')
        links.new(principled.outputs['BSDF'], output.inputs['Surface'])
        
        # Find image textures
        # Re-create image texture nodes from the loaded images
        color_images = [img for img in bpy.data.images 
                       if img.name != 'Render Result' and img.size[0] > 100
                       and img.colorspace_settings.name == 'sRGB']
        non_color_images = [img for img in bpy.data.images 
                           if img.name != 'Render Result' and img.size[0] > 100
                           and img.colorspace_settings.name != 'sRGB']
        
        log(f"  Color images: {len(color_images)}, Non-color: {len(non_color_images)}")
        
        if color_images:
            # Use first color image as base color
            tex = nodes.new('ShaderNodeTexImage')
            tex.image = color_images[0]
            tex.label = "Albedo"
            links.new(tex.outputs['Color'], principled.inputs['Base Color'])
            log(f"  Connected {color_images[0].name} → Base Color (sRGB)")
        
        if len(color_images) > 1:
            # Second color image → Metallic or Emission
            tex2 = nodes.new('ShaderNodeTexImage')
            tex2.image = color_images[1]
            # Try as emission (some GLB store ORM in separate channels)
            links.new(tex2.outputs['Color'], principled.inputs['Emission Color'])
            links.new(tex2.outputs['Color'], principled.inputs['Emission Strength'])
            principled.inputs['Emission Strength'].default_value = 0.0
            log(f"  Connected {color_images[1].name} → Emission (off)")
        
        if non_color_images:
            # First non-color → Roughness
            rough_tex = nodes.new('ShaderNodeTexImage')
            rough_tex.image = non_color_images[0]
            rough_tex.label = "Roughness"
            links.new(rough_tex.outputs['Color'], principled.inputs['Roughness'])
            log(f"  Connected {non_color_images[0].name} → Roughness")
            
            # Second non-color → Normal via Normal Map
            if len(non_color_images) > 1:
                normal_tex = nodes.new('ShaderNodeTexImage')
                normal_tex.image = non_color_images[1]
                normal_tex.label = "Normal"
                normal_map = nodes.new('ShaderNodeNormalMap')
                links.new(normal_tex.outputs['Color'], normal_map.inputs['Color'])
                links.new(normal_map.outputs['Normal'], principled.inputs['Normal'])
                log(f"  Connected {non_color_images[1].name} → Normal Map")
        
        # Set some defaults for better rendering
        principled.inputs['Roughness'].default_value = 0.5
        principled.inputs['Specular IOR Level'].default_value = 0.5
    
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.shade_smooth()
    bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')
    obj.location = (0, 0, 0)

    # Lighting
    key = bpy.data.lights.new(name="Key", type='AREA')
    key.energy = 80000; key.size = 5; key.color = (1.0, 0.95, 0.9)
    key_obj = bpy.data.objects.new("Key", key)
    bpy.context.collection.objects.link(key_obj)
    key_obj.location = (4, -4, 5); key_obj.rotation_euler = (math.radians(50), 0, math.radians(45))

    fill = bpy.data.lights.new(name="Fill", type='AREA')
    fill.energy = 30000; fill.size = 5; fill.color = (0.7, 0.8, 1.0)
    fill_obj = bpy.data.objects.new("Fill", fill)
    bpy.context.collection.objects.link(fill_obj)
    fill_obj.location = (-4, -3, 3); fill_obj.rotation_euler = (math.radians(55), 0, math.radians(-35))

    rim = bpy.data.lights.new(name="Rim", type='AREA')
    rim.energy = 50000; rim.size = 5; rim.color = (1.0, 0.8, 0.6)
    rim_obj = bpy.data.objects.new("Rim", rim)
    bpy.context.collection.objects.link(rim_obj)
    rim_obj.location = (0, 6, 4); rim_obj.rotation_euler = (math.radians(130), 0, 0)

    world = bpy.context.scene.world
    if not world:
        world = bpy.data.worlds.new("World")
        bpy.context.scene.world = world
    world.use_nodes = True
    bg = world.node_tree.nodes.get("Background")
    if bg:
        bg.inputs[0].default_value = (0.02, 0.02, 0.04, 1.0)
        bg.inputs[1].default_value = 1.0

    cam_data = bpy.data.cameras.new("Cam")
    cam = bpy.data.objects.new("Camera", cam_data)
    bpy.context.collection.objects.link(cam)
    cam.location = (5, -5, 2.5); cam.rotation_euler = (math.radians(72), 0, math.radians(45))
    bpy.context.scene.camera = cam

    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.samples = 32
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 768; scene.render.resolution_y = 768
    scene.render.image_settings.file_format = 'PNG'
    scene.view_settings.view_transform = 'Filmic'
    scene.view_settings.look = 'Medium High Contrast'

    scene.render.filepath = os.path.join(OUTPUT_DIR, f"{output_name}_3quarter.png")
    log(f"  Rendering {output_name} 3/4...")
    bpy.ops.render.render(write_still=True)

    cam.location = (0, -7, 2); cam.rotation_euler = (math.radians(78), 0, 0)
    scene.render.filepath = os.path.join(OUTPUT_DIR, f"{output_name}_front.png")
    log(f"  Rendering {output_name} front...")
    bpy.ops.render.render(write_still=True)

    return True

def main():
    log("=" * 50)
    log("ECLIPSE RENDER — sRGB FORCED")
    log("=" * 50)

    input_dir = r"C:\Users\Tap\EclipseRotF\Content\Characters"
    glb_files = sorted(glob.glob(os.path.join(input_dir, "*_HQ.glb")))

    # Test with Aetherborn Male only
    test = [g for g in glb_files if "AETHERBORN_MALE" in os.path.basename(g)]
    
    for glb in test:
        name = os.path.splitext(os.path.basename(glb))[0]
        log(f"\n[{name}]")
        try:
            fix_and_render(glb, name)
        except Exception as e:
            log(f"  Error: {e}")

    log(f"\nDone! Check: {OUTPUT_DIR}")

if __name__ == "__main__":
    main()