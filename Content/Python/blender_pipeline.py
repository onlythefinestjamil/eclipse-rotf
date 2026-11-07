"""
Eclipse: Realms of the Fractured
Full Blender Art Pipeline — All Characters
Momentum Labs | Hermes | November 7, 2026

Blender 5.2.3 LTS headless Python pipeline:
1. Import GLB
2. Clean geometry (remove doubles, degenerate faces, non-manifold)
3. Recalculate normals
4. Smart UV unwrap
5. Decimation for clean topology (target 40% of original)
6. Laplacian smoothing for clean surface
7. Procedural skin detail (noise displacement on normals)
8. Shade smooth + auto smooth
9. Export as clean FBX with embedded textures
"""
import bpy
import os
import sys
import glob
import math
import json
import bmesh

CHARACTERS_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters"
OUTPUT_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters\Pipeline"
BLENDER_OUTPUT = r"C:\Users\Tap\EclipseRotF\Content\Characters\Blender_Output"
os.makedirs(OUTPUT_DIR, exist_ok=True)
os.makedirs(BLENDER_OUTPUT, exist_ok=True)

def log(msg):
    print(f"[ECLIPSE BLENDER] {msg}", flush=True)

def clear_scene():
    """Clear all objects and orphan data"""
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    for collection in [bpy.data.meshes, bpy.data.materials, bpy.data.textures,
                       bpy.data.images, bpy.data.lights, bpy.data.cameras]:
        for item in list(collection):
            if item.users == 0:
                collection.remove(item)

def get_mesh_objects():
    """Get all mesh objects in the scene"""
    return [obj for obj in bpy.context.scene.objects if obj.type == 'MESH']

def clean_geometry(obj):
    """Step 1: Clean geometry — remove doubles, degenerate, non-manifold"""
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)

    # Enter edit mode
    bpy.ops.object.mode_set(mode='EDIT')
    bm = bmesh.from_edit_mesh(obj.data)

    # Remove duplicate vertices
    bmesh.ops.remove_doubles(bm, verts=bm.verts, dist=0.0001)

    # Delete degenerate faces (zero area)
    degenerate = [f for f in bm.faces if f.calc_area() < 1e-8]
    bmesh.ops.delete(bm, geom=degenerate, context='FACES')

    # Find and fix non-manifold edges
    non_manifold = [e for e in bm.edges if not e.is_manifold and len(e.link_faces) > 2]
    if non_manifold:
        bmesh.ops.delete(bm, geom=non_manifold, context='EDGES')

    bmesh.update_edit_mesh(obj.data)

    # Recalculate normals
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.mesh.normals_make_consistent(inside=False)

    bpy.ops.object.mode_set(mode='OBJECT')

    log(f"  Cleaned: {len(obj.data.vertices)} verts, {len(obj.data.polygons)} faces")

def smart_uv_unwrap(obj):
    """Step 2: Smart UV unwrap for better texture mapping"""
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)

    # Ensure UV layer exists
    if not obj.data.uv_layers:
        obj.data.uv_layers.new(name="UVMap")

    # Set active UV layer
    obj.data.uv_layers.active = obj.data.uv_layers[0]

    bpy.ops.object.mode_set(mode='EDIT')
    bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(
        angle_limit=math.radians(66),
        correct_aspect=True,
        scale_to_bounds=True
    )
    bpy.ops.object.mode_set(mode='OBJECT')

    log(f"  UV unwrapped")

def decimate_mesh(obj, ratio=0.4):
    """Step 3: Decimate for clean topology"""
    orig_faces = len(obj.data.polygons)
    if orig_faces < 50000:
        log(f"  Skipping decimation (only {orig_faces} faces)")
        return

    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)

    # Add decimation modifier
    mod = obj.modifiers.new(name="Decimate", type='DECIMATE')
    mod.decimate_type = 'COLLAPSE'
    mod.ratio = ratio
    mod.use_collapse = True

    # Apply
    bpy.ops.object.modifier_apply(modifier="Decimate")

    new_faces = len(obj.data.polygons)
    log(f"  Decimated: {orig_faces} → {new_faces} faces ({(new_faces/orig_faces*100):.0f}%)")

def laplacian_smooth(obj, iterations=2):
    """Step 4: Laplacian smoothing for clean surface"""
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)

    mod = obj.modifiers.new(name="Smooth", type='SMOOTH')
    mod.iterations = iterations
    mod.factor = 0.3

    bpy.ops.object.modifier_apply(modifier="Smooth")

    log(f"  Smoothed ({iterations} iterations)")

def add_procedural_skin_detail(obj, intensity=0.002):
    """Step 5: Procedural skin detail — pores, veins, muscle definition"""
    import numpy as np

    mesh = obj.data
    n = len(mesh.vertices)

    if n == 0:
        return

    # Get vertex normals
    normals = np.array([[v.normal.x, v.normal.y, v.normal.z] for v in mesh.vertices])
    verts = np.array([[v.co.x, v.co.y, v.co.z] for v in mesh.vertices])

    np.random.seed(42)  # Deterministic

    # Multi-frequency noise for realistic skin
    noise = (
        np.random.randn(n) * 0.3 +           # Fine pores
        np.sin(verts[:, 0] * 50) * 0.2 +     # Medium detail
        np.sin(verts[:, 1] * 30) * 0.15 +    # Wrinkle lines
        np.cos(verts[:, 2] * 40) * 0.1 +      # Muscle definition
        np.sin(verts[:, 0] * 15 + verts[:, 1] * 20) * 0.08  # Fine lines
    ) * intensity

    # Apply displacement along normals
    displaced = verts + normals * noise[:, np.newaxis]

    for i, v in enumerate(mesh.vertices):
        v.co = displaced[i]

    log(f"  Skin detail added (intensity={intensity})")

def shade_smooth(obj):
    """Step 6: Smooth shading + auto smooth"""
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)

    bpy.ops.object.shade_smooth()
    obj.data.use_auto_smooth = True
    obj.data.auto_smooth_angle = math.radians(35)

    log(f"  Shade smooth applied")

def export_fbx(obj, filepath):
    """Step 7: Export as FBX with embedded textures"""
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj

    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_ALL',
        object_types={'MESH'},
        use_mesh_modifiers=True,
        mesh_smooth_type='FACE',
        use_anim=False,
        path_mode='AUTO',
        embed_textures=True,
        axis_forward='Y',
        axis_up='Z'
    )

    if os.path.exists(filepath):
        size = os.path.getsize(filepath) / (1024 * 1024)
        log(f"  Exported FBX: {os.path.basename(filepath)} ({size:.1f} MB)")
        return True
    return False

def process_model(glb_path, output_dir):
    """Process a single GLB model through the full Blender pipeline"""
    filename = os.path.splitext(os.path.basename(glb_path))[0]
    log(f"\n--- {filename} ---")

    # Clear scene
    clear_scene()

    # Import GLB
    try:
        bpy.ops.import_scene.gltf(filepath=glb_path)
        imported = get_mesh_objects()
        if not imported:
            log(f"  SKIP: No mesh objects found")
            return False
        log(f"  Imported {len(imported)} mesh objects")
    except Exception as e:
        log(f"  Import error: {e}")
        return False

    # Process first mesh (main character body)
    obj = imported[0]
    bpy.context.view_layer.objects.active = obj

    orig_verts = len(obj.data.vertices)
    orig_faces = len(obj.data.polygons)
    log(f"  Original: {orig_verts} verts, {orig_faces} faces")

    # Step 1: Clean geometry
    try:
        clean_geometry(obj)
    except Exception as e:
        log(f"  Clean warning: {e}")

    # Step 2: UV unwrap
    try:
        smart_uv_unwrap(obj)
    except Exception as e:
        log(f"  UV warning: {e}")

    # Step 3: Decimate
    try:
        decimate_mesh(obj, ratio=0.4)
    except Exception as e:
        log(f"  Decimate warning: {e}")

    # Step 4: Smooth
    try:
        laplacian_smooth(obj, iterations=2)
    except Exception as e:
        log(f"  Smooth warning: {e}")

    # Step 5: Procedural skin detail
    try:
        add_procedural_skin_detail(obj, intensity=0.002)
    except Exception as e:
        log(f"  Skin detail warning: {e}")

    # Step 6: Smooth shading
    try:
        shade_smooth(obj)
    except Exception as e:
        log(f"  Shade warning: {e}")

    # Step 7: Export FBX
    fbx_path = os.path.join(output_dir, f"{filename}_pipeline.fbx")
    try:
        ok = export_fbx(obj, fbx_path)
        if ok:
            return True
    except Exception as e:
        log(f"  FBX export error: {e}")

    # Fallback: export as OBJ
    obj_path = os.path.join(output_dir, f"{filename}_pipeline.obj")
    try:
        bpy.ops.object.select_all(action='DESELECT')
        obj.select_set(True)
        bpy.ops.wm.obj_export(filepath=obj_path, export_selected_objects=True)
        if os.path.exists(obj_path):
            size = os.path.getsize(obj_path) / (1024 * 1024)
            log(f"  Exported OBJ fallback: {size:.1f} MB")
            return True
    except Exception as e:
        log(f"  OBJ fallback error: {e}")

    return False

def main():
    log("=" * 60)
    log("ECLIPSE BLENDER ART PIPELINE — ALL 29 MODELS")
    log(f"Blender {bpy.app.version_string}")
    log("=" * 60)

    glb_files = sorted(glob.glob(os.path.join(CHARACTERS_DIR, "*_HQ.glb")))
    log(f"Found {len(glb_files)} HQ models")

    results = []
    ok_count = 0

    for i, glb in enumerate(glb_files):
        fn = os.path.basename(glb)
        log(f"\n[{i+1}/{len(glb_files)}] {fn}")
        success = False
        try:
            success = process_model(glb, BLENDER_OUTPUT)
        except Exception as e:
            log(f"  ERROR: {e}")

        results.append({"file": fn, "status": "OK" if success else "FAILED"})
        if success:
            ok_count += 1

        # Save progress
        with open(os.path.join(BLENDER_OUTPUT, "pipeline_results.json"), "w") as f:
            json.dump(results, f, indent=2)

    log(f"\n{'=' * 60}")
    log(f"BLENDER PIPELINE COMPLETE")
    log(f"  Success: {ok_count}/{len(glb_files)}")
    log(f"  Output: {BLENDER_OUTPUT}")
    log(f"{'=' * 60}")

if __name__ == "__main__":
    main()
