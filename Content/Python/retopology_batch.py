"""
Eclipse: Realms of the Fractured
Batch Retopology + UV Cleanup Pipeline
Momentum Labs | Hermes | November 7, 2026

Processes all GLB models through Blender Python API:
1. Import GLB
2. Decimate/remesh for clean topology
3. Fix UV maps
4. Generate proper normals
5. Export as clean FBX with materials

Usage: blender --background --python retopology_batch.py
"""
import bpy
import os
import sys
import glob
import math

CHARACTERS_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters"
OUTPUT_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters\Retopo"

def log(msg):
    print(f"[ECLIPSE RETOPO] {msg}", flush=True)

def clear_scene():
    """Clear all objects from the scene"""
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    # Clean orphan data
    for collection in [bpy.data.meshes, bpy.data.materials, bpy.data.textures, bpy.data.images]:
        for item in list(collection):
            if item.users == 0:
                collection.remove(item)

def import_glb(filepath):
    """Import a GLB file into Blender"""
    try:
        bpy.ops.import_scene.gltf(filepath=filepath)
        imported = bpy.context.selected_objects
        log(f"  Imported {len(imported)} objects from {os.path.basename(filepath)}")
        return imported
    except Exception as e:
        log(f"  ERROR importing {filepath}: {e}")
        return []

def select_mesh_objects():
    """Select all mesh objects in the scene"""
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH']
    for obj in meshes:
        obj.select_set(True)
    return meshes

def decimate_mesh(obj, ratio=0.3):
    """Apply decimation modifier to reduce poly count while preserving shape"""
    if obj.type != 'MESH':
        return
    
    # Add decimation modifier
    mod = obj.modifiers.new(name="Decimate", type='DECIMATE')
    mod.ratio = ratio
    mod.use_collapse = True
    
    # Apply modifier
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier="Decimate")
    
    verts = len(obj.data.vertices)
    faces = len(obj.data.polygons)
    log(f"  Decimated {obj.name}: {verts} verts, {faces} faces")

def remesh_object(obj, voxel_size=0.05):
    """Apply voxel remesh for clean topology"""
    if obj.type != 'MESH':
        return
    
    # Voxel remesh for clean topology
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    
    remesh_mod = obj.modifiers.new(name="Remesh", type='REMESH')
    remesh_mod.mode = 'VOXEL'
    remesh_mod.voxel_size = voxel_size
    remesh_mod.use_smooth_shade = True
    
    bpy.ops.object.modifier_apply(modifier="Remesh")
    
    verts = len(obj.data.vertices)
    log(f"  Remeshed {obj.name}: {verts} verts")

def fix_uvs(obj):
    """Fix UV maps — ensure proper UV coordinates"""
    if obj.type != 'MESH':
        return
    
    mesh = obj.data
    
    # Ensure UV layer exists
    if not mesh.uv_layers:
        mesh.uv_layers.new(name="UVMap")
    
    uv_layer = mesh.uv_layers.active
    if uv_layer:
        # Smart UV unwrap for better texture mapping
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        try:
            bpy.ops.object.mode_set(mode='EDIT')
            bpy.ops.mesh.select_all(action='SELECT')
            bpy.ops.uv.smart_project(angle_limit=math.radians(66))
            bpy.ops.object.mode_set(mode='OBJECT')
        except Exception as e:
            log(f"  UV fix warning on {obj.name}: {e}")

def compute_normals(obj):
    """Recalculate normals to face outward"""
    if obj.type != 'MESH':
        return
    
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    try:
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.mesh.normals_make_consistent(inside=False)
        bpy.ops.object.mode_set(mode='OBJECT')
        log(f"  Normals recalculated for {obj.name}")
    except Exception as e:
        log(f"  Normal calc warning on {obj.name}: {e}")

def merge_materials(obj):
    """Ensure materials are properly assigned"""
    if obj.type != 'MESH':
        return
    
    # Ensure each material slot has a material
    for slot in obj.material_slots:
        if not slot.material:
            mat = bpy.data.materials.new(name=f"{obj.name}_mat")
            slot.material = mat

def export_fbx(obj, output_path):
    """Export object as FBX with proper settings"""
    # Select only this object
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    
    # Export FBX
    bpy.ops.export_scene.fbx(
        filepath=output_path,
        use_selection=True,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_ALL',
        use_space_transform=True,
        bake_space_transform=True,
        object_types={'MESH', 'ARMATURE'},
        use_mesh_modifiers=True,
        use_mesh_modifiers_render=True,
        mesh_smooth_type='FACE',
        use_anim=False,
        use_armature_skin_deform=True,
        bake_anim=False,
        path_mode='AUTO',
        embed_textures=True,
    )
    log(f"  Exported FBX: {os.path.basename(output_path)}")

def process_model(glb_path, output_dir):
    """Process a single GLB model through the full pipeline"""
    filename = os.path.splitext(os.path.basename(glb_path))[0]
    log(f"\n--- Processing: {filename} ---")
    
    # Clear scene
    clear_scene()
    
    # Import
    objects = import_glb(glb_path)
    if not objects:
        log(f"  SKIP: No objects imported from {glb_path}")
        return False
    
    # Process each mesh
    meshes = select_mesh_objects()
    if not meshes:
        log(f"  SKIP: No mesh objects found")
        return False
    
    for obj in meshes:
        log(f"  Processing mesh: {obj.name}")
        
        # Get original vert count
        orig_verts = len(obj.data.vertices)
        
        # Decimate (keep 30% of polys for clean topology)
        if orig_verts > 10000:
            decimate_mesh(obj, ratio=0.3)
        
        # Fix UVs
        fix_uvs(obj)
        
        # Recalculate normals
        compute_normals(obj)
        
        # Fix materials
        merge_materials(obj)
    
    # Export as FBX
    output_path = os.path.join(output_dir, f"{filename}_retopo.fbx")
    export_fbx(meshes[0], output_path)
    
    if os.path.exists(output_path):
        size_mb = os.path.getsize(output_path) / (1024 * 1024)
        log(f"  SUCCESS: {filename}_retopo.fbx ({size_mb:.1f} MB)")
        return True
    else:
        log(f"  FAILED: Export did not produce file")
        return False

def main():
    log("=" * 60)
    log("ECLIPSE BATCH RETOPOLOGY PIPELINE")
    log("=" * 60)
    
    # Create output directory
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    # Find all HQ GLB files
    glb_files = sorted(glob.glob(os.path.join(CHARACTERS_DIR, "*_HQ.glb")))
    log(f"Found {len(glb_files)} HQ GLB files to process")
    
    success_count = 0
    fail_count = 0
    
    for i, glb in enumerate(glb_files):
        log(f"\n[{i+1}/{len(glb_files)}] {os.path.basename(glb)}")
        try:
            if process_model(glb, OUTPUT_DIR):
                success_count += 1
            else:
                fail_count += 1
        except Exception as e:
            log(f"  ERROR: {e}")
            fail_count += 1
    
    log(f"\n{'=' * 60}")
    log(f"PIPELINE COMPLETE")
    log(f"  Success: {success_count}")
    log(f"  Failed: {fail_count}")
    log(f"  Output: {OUTPUT_DIR}")
    log(f"{'=' * 60}")

if __name__ == "__main__":
    main()
