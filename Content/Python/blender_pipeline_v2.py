"""
Eclipse: Realms of the Fractured
Blender Art Pipeline — Memory-Safe Version
December 7, 2026

Fixed: Decimate FIRST (modifier-based, no bmesh), then clean.
Processes models one at a time to avoid memory issues.
"""
import bpy
import os
import sys
import glob
import math
import json
import numpy as np

CHARACTERS_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters"
OUTPUT_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters\Blender_Output"

def log(msg):
    print(f"[ECLIPSE] {msg}", flush=True)

def clear_scene():
    """Aggressive clear — free all memory"""
    # Delete all objects
    for obj in list(bpy.data.objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    # Purge orphan data
    for collection in [bpy.data.meshes, bpy.data.materials, bpy.data.textures,
                       bpy.data.images, bpy.data.lights, bpy.data.cameras]:
        for item in list(collection):
            if item.users == 0:
                collection.remove(item)
    # Force garbage collection
    import gc
    gc.collect()

def process_model(glb_path, output_dir):
    """Process a single GLB — decimate first, then clean"""
    filename = os.path.splitext(os.path.basename(glb_path))[0]
    log(f"--- {filename} ---")

    # Clear scene
    clear_scene()

    # Import GLB
    try:
        bpy.ops.import_scene.gltf(filepath=glb_path)
    except Exception as e:
        log(f"  Import error: {e}")
        return False

    # Find mesh objects
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH']
    if not meshes:
        log(f"  No meshes found")
        return False

    obj = meshes[0]
    orig_verts = len(obj.data.vertices)
    orig_faces = len(obj.data.polygons)
    log(f"  Original: {orig_verts} verts, {orig_faces} faces")

    # Step 1: DECIMATE FIRST (modifier-based, no bmesh, memory-safe)
    if orig_faces > 50000:
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)

        mod = obj.modifiers.new(name="Decimate", type='DECIMATE')
        mod.decimate_type = 'COLLAPSE'
        mod.ratio = 0.35  # Keep 35%

        try:
            bpy.ops.object.modifier_apply(modifier="Decimate")
            new_faces = len(obj.data.polygons)
            log(f"  Decimated: {orig_faces} -> {new_faces} faces")
        except Exception as e:
            log(f"  Decimate error: {e}")
            obj.modifiers.remove(mod)

    # Step 2: Remove doubles (operator-based, not bmesh)
    try:
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.mesh.remove_doubles(threshold=0.0001)
        bpy.ops.mesh.normals_make_consistent(inside=False)
        bpy.ops.object.mode_set(mode='OBJECT')
        log(f"  Cleaned doubles + normals: {len(obj.data.vertices)} verts")
    except Exception as e:
        log(f"  Clean warning: {e}")
        try:
            bpy.ops.object.mode_set(mode='OBJECT')
        except:
            pass

    # Step 3: Smooth shading
    try:
        bpy.ops.object.shade_smooth()
        obj.data.use_auto_smooth = True
        obj.data.auto_smooth_angle = math.radians(35)
        log(f"  Shade smooth")
    except:
        pass

    # Step 4: UV unwrap (only if low enough poly)
    if len(obj.data.polygons) < 500000:
        try:
            if not obj.data.uv_layers:
                obj.data.uv_layers.new(name="UVMap")
            obj.data.uv_layers.active = obj.data.uv_layers[0]
            bpy.ops.object.mode_set(mode='EDIT')
            bpy.ops.mesh.select_all(action='SELECT')
            bpy.ops.uv.smart_project(angle_limit=math.radians(66), correct_aspect=True, scale_to_bounds=True)
            bpy.ops.object.mode_set(mode='OBJECT')
            log(f"  UV unwrapped")
        except Exception as e:
            log(f"  UV warning: {e}")
            try:
                bpy.ops.object.mode_set(mode='OBJECT')
            except:
                pass

    # Step 5: Procedural skin detail (numpy-based, memory-safe)
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
            log(f"  Skin detail added")
    except Exception as e:
        log(f"  Skin detail skipped: {e}")

    # Step 6: Export as GLB (preserves materials + textures)
    glb_path = os.path.join(output_dir, f"{filename}_blender.glb")
    try:
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        bpy.ops.export_scene.gltf(
            filepath=glb_path,
            use_selection=True,
            export_format='GLB',
            export_materials='EXPORT',
            export_textures='EXPORT',
            export_apply=True
        )
        if os.path.exists(glb_path):
            size = os.path.getsize(glb_path) / (1024 * 1024)
            log(f"  Exported GLB: {size:.1f} MB")
            return True
    except Exception as e:
        log(f"  GLB export error: {e}")

    # Fallback: OBJ export
    obj_path = os.path.join(output_dir, f"{filename}_blender.obj")
    try:
        bpy.ops.wm.obj_export(filepath=obj_path, export_selected_objects=True, export_materials=True)
        if os.path.exists(obj_path):
            size = os.path.getsize(obj_path) / (1024 * 1024)
            log(f"  Exported OBJ: {size:.1f} MB")
            return True
    except Exception as e:
        log(f"  OBJ export error: {e}")

    return False

def main():
    log("=" * 60)
    log("ECLIPSE BLENDER PIPELINE — MEMORY-SAFE")
    log(f"Blender {bpy.app.version_string}")
    log("=" * 60)

    os.makedirs(OUTPUT_DIR, exist_ok=True)

    glb_files = sorted(glob.glob(os.path.join(CHARACTERS_DIR, "*_HQ.glb")))
    log(f"Found {len(glb_files)} HQ models")

    # Skip already-processed files
    existing = set(os.listdir(OUTPUT_DIR))
    to_process = []
    for glb in glb_files:
        fn = os.path.splitext(os.path.basename(glb))[0]
        glb_out = f"{fn}_blender.glb"
        obj_out = f"{fn}_blender.obj"
        if glb_out in existing or obj_out in existing:
            log(f"  Skipping {fn} (already processed)")
        else:
            to_process.append(glb)

    log(f"To process: {len(to_process)}")
    if not to_process:
        log("All models already processed!")
        return

    ok_count = 0
    fail_count = 0
    results = []

    for i, glb in enumerate(to_process):
        fn = os.path.basename(glb)
        log(f"\n[{i+1}/{len(to_process)}] {fn}")
        success = False
        try:
            success = process_model(glb, OUTPUT_DIR)
        except Exception as e:
            log(f"  FATAL: {e}")

        if success:
            ok_count += 1
            results.append({"file": fn, "status": "OK"})
        else:
            fail_count += 1
            results.append({"file": fn, "status": "FAILED"})

        # Save progress
        with open(os.path.join(OUTPUT_DIR, "pipeline_results.json"), "w") as f:
            json.dump(results, f, indent=2)

    log(f"\n{'=' * 60}")
    log(f"COMPLETE: {ok_count} OK, {fail_count} FAILED")
    log(f"{'=' * 60}")

if __name__ == "__main__":
    main()
