"""
Eclipse: AAA Enhancement — Memory Safe
No subdivision (crashes Blender). Just displacement, bevel, sculpt, PBR.
"""
import bpy
import os
import math
import numpy as np

CHARACTERS_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters"
OUTPUT_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters\AAA_Enhanced"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def log(msg):
    print(f"[AAA] {msg}", flush=True)

def clear_scene():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for coll in [bpy.data.meshes, bpy.data.materials, bpy.data.lights, bpy.data.cameras, bpy.data.images]:
        for item in list(coll):
            if item.users == 0:
                coll.remove(item)

def enhance(glb_path, output_name):
    clear_scene()
    try:
        bpy.ops.import_scene.gltf(filepath=glb_path)
    except Exception as e:
        log(f"  Import failed: {e}")
        return False

    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    if not meshes:
        return False

    obj = meshes[0]
    log(f"  Original: {len(obj.data.vertices)} verts, {len(obj.data.polygons)} faces")
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)

    # Step 1: Decimate to manageable size
    if len(obj.data.polygons) > 300000:
        decim = obj.modifiers.new(name='Decimate', type='DECIMATE')
        decim.decimate_type = 'COLLAPSE'
        decim.ratio = 0.3
        try:
            bpy.ops.object.modifier_apply(modifier='Decimate')
            log(f"  Decimated: {len(obj.data.vertices)} verts")
        except:
            obj.modifiers.remove(decim)

    # Step 2: Smooth shading (no subdivision — avoids crash)
    bpy.ops.object.shade_smooth()
    try:
        obj.data.use_auto_smooth = True
        obj.data.auto_smooth_angle = math.radians(30)
    except:
        pass
    log(f"  Smooth shading applied")

    # Step 3: Bevel for cleaner edges
    try:
        bevel = obj.modifiers.new(name='Bevel', type='BEVEL')
        bevel.width = 0.001
        bevel.segments = 2
        bevel.limit_method = 'ANGLE'
        bevel.angle_limit = math.radians(30)
        bpy.ops.object.modifier_apply(modifier='Bevel')
        log(f"  Beveled edges")
    except:
        try: obj.modifiers.remove(bevel)
        except: pass

    # Step 4: Sculpt detail pass (vertex displacement — no crash risk)
    try:
        mesh = obj.data
        n = len(mesh.vertices)
        if n > 0 and n < 2000000:
            verts = np.array([[v.co.x, v.co.y, v.co.z] for v in mesh.vertices])
            normals = np.array([[v.normal.x, v.normal.y, v.normal.z] for v in mesh.vertices])
            np.random.seed(42)
            noise = (
                np.random.randn(n) * 0.3 +
                np.sin(verts[:, 0] * 50) * 0.2 +
                np.sin(verts[:, 1] * 30) * 0.15 +
                np.cos(verts[:, 2] * 40) * 0.1
            ) * 0.002
            displaced = verts + normals * noise[:, np.newaxis]
            for i, v in enumerate(mesh.vertices):
                v.co = displaced[i].tolist()
            log(f"  Sculpt detail: {n} verts")
    except Exception as e:
        log(f"  Sculpt detail skipped: {e}")

    # Step 5: Enhanced PBR materials
    for slot in obj.material_slots:
        if not slot.material:
            continue
        mat = slot.material
        mat.use_nodes = True
        nodes = mat.node_tree.nodes
        links = mat.node_tree.links
        principled = None
        for node in nodes:
            if node.type == 'BSDF_PRINCIPLED':
                principled = node
                break
        if principled:
            principled.inputs['Roughness'].default_value = 0.45
            # Add noise bump for surface micro-detail
            noise = nodes.new('ShaderNodeTexNoise')
            noise.inputs['Scale'].default_value = 50.0
            noise.inputs['Detail'].default_value = 8.0
            bump = nodes.new('ShaderNodeBump')
            bump.inputs['Strength'].default_value = 0.05
            links.new(noise.outputs['Fac'], bump.inputs['Height'])
            links.new(bump.outputs['Normal'], principled.inputs['Normal'])
            log(f"  PBR: {mat.name}")

    # Step 6: Export GLB
    glb_out = os.path.join(OUTPUT_DIR, f"{output_name}_enhanced.glb")
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    try:
        bpy.ops.export_scene.gltf(filepath=glb_out, use_selection=True, export_format='GLB', export_materials='EXPORT', export_apply=True)
        if os.path.exists(glb_out):
            size = os.path.getsize(glb_out) / (1024*1024)
            log(f"  Exported GLB: {size:.1f} MB")
            return True
    except Exception as e:
        log(f"  GLB error: {e}")

    # Fallback: FBX
    fbx_out = os.path.join(OUTPUT_DIR, f"{output_name}_enhanced.fbx")
    try:
        bpy.ops.export_scene.fbx(filepath=fbx_out, use_selection=True, mesh_smooth_type='FACE', path_mode='AUTO')
        if os.path.exists(fbx_out):
            size = os.path.getsize(fbx_out) / (1024*1024)
            log(f"  Exported FBX: {size:.1f} MB")
            return True
    except:
        pass

    return False

log("=" * 50)
log("AAA ENHANCEMENT — MEMORY SAFE (NO SUBDIVISION)")
log("=" * 50)

import glob
glb_files = sorted(glob.glob(os.path.join(CHARACTERS_DIR, "*_HQ.glb")))

# Add the AAA model first
aaa_glb = os.path.join(CHARACTERS_DIR, "AETHERBORN_MALE_AAA.glb")
if os.path.exists(aaa_glb):
    glb_files.insert(0, aaa_glb)

log(f"Processing {len(glb_files)} models...")

ok = 0
fail = 0
import json
results = []

for i, glb in enumerate(glb_files):
    fn = os.path.basename(glb)
    name = os.path.splitext(fn)[0]
    log(f"\n[{i+1}/{len(glb_files)}] {fn}")
    success = False
    try:
        success = enhance(glb, name)
    except Exception as e:
        log(f"  FATAL: {e}")
    if success:
        ok += 1
        results.append({"file": fn, "status": "OK"})
    else:
        fail += 1
        results.append({"file": fn, "status": "FAILED"})
    with open(os.path.join(OUTPUT_DIR, "aaa_safe_results.json"), "w") as f:
        json.dump(results, f, indent=2)

log(f"\n{'=' * 50}")
log(f"COMPLETE: {ok} OK, {fail} FAILED")
log(f"{'=' * 50}")
