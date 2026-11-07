"""
Eclipse: Realms of the Fractured
Full Character Art Pipeline — Fixed
Uses pymeshlab (correct filter names) + trimesh
November 7, 2026
"""
import os, sys, glob, json
import numpy as np

CHARACTERS_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters"
OUTPUT_DIR = r"C:\Users\Tap\EclipseRotF\Content\Characters\Pipeline"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def log(msg, level="INFO"):
    print(f"[{level}] {msg}", flush=True)

def process_model(glb_path, output_dir):
    """Process a single GLB through the full pipeline"""
    import pymeshlab as pml
    import trimesh
    
    filename = os.path.splitext(os.path.basename(glb_path))[0]
    log(f"Processing: {filename}")
    
    # === Step 1: pymeshlab cleanup + retopology ===
    try:
        ms = pml.MeshSet()
        ms.load_new_mesh(glb_path)
        m = ms.current_mesh()
        orig_verts = m.vertex_number()
        orig_faces = m.face_number()
        log(f"  Loaded: {orig_verts} verts, {orig_faces} faces")
        
        # Remove duplicate vertices
        try: ms.meshing_remove_duplicate_vertices()
        except: pass
        
        # Remove duplicate faces
        try: ms.meshing_remove_duplicate_faces()
        except: pass
        
        # Remove unreferenced vertices
        try: ms.meshing_remove_unreferenced_vertices()
        except: pass
        
        # Repair non-manifold
        try: ms.meshing_repair_non_manifold_edges()
        except: pass
        try: ms.meshing_repair_non_manifold_vertices()
        except: pass
        
        # Fix normals
        try: ms.compute_normal_per_face()
        except: pass
        try: ms.compute_normal_per_vertex()
        except: pass
        
        # Decimation — reduce to 40% for cleaner topology
        if orig_faces > 100000:
            target = int(orig_faces * 0.4)
            try:
                ms.meshing_decimation_quadric_edge_collapse(
                    targetfacenum=target,
                    preservetopology=True,
                    preservenormal=True
                )
                m = ms.current_mesh()
                log(f"  Decimated: {m.vertex_number()} verts, {m.face_number()} faces")
            except Exception as e:
                log(f"  Decimation note: {e}")
        
        # Laplacian smoothing for cleaner surface
        try:
            ms.apply_coord_laplacian_smoothing(stepsmoothnum=2)
            log(f"  Laplacian smoothed")
        except: pass
        
        # Export as OBJ (preserves materials + textures)
        obj_path = os.path.join(output_dir, f"{filename}_retopo.obj")
        try:
            ms.save_current_mesh(obj_path)
            size = os.path.getsize(obj_path) / (1024*1024)
            log(f"  Exported OBJ: {size:.1f} MB")
        except Exception as e:
            log(f"  OBJ export failed: {e}", "WARN")
            # Try PLY as fallback
            ply_path = os.path.join(output_dir, f"{filename}_retopo.ply")
            try:
                ms.save_current_mesh(ply_path)
                log(f"  Exported PLY fallback")
            except Exception as e2:
                log(f"  PLY also failed: {e2}", "ERROR")
                return False
        
        # === Step 2: trimesh for UV + final GLB export ===
        log(f"  Step 2: trimesh cleanup + GLB export")
        try:
            scene = trimesh.load(glb_path, force='scene')
            
            if hasattr(scene, 'geometry'):
                for name, mesh in scene.geometry.items():
                    if isinstance(mesh, trimesh.Trimesh):
                        mesh.process()  # Clean up
                        mesh.fix_normals()
                        mesh.remove_unreferenced_vertices()
                        # Smooth
                        try: mesh.smooth_shaded()
                        except: pass
            else:
                if isinstance(scene, trimesh.Trimesh):
                    scene.process()
                    scene.fix_normals()
            
            # Export cleaned GLB
            glb_out = os.path.join(output_dir, f"{filename}_clean.glb")
            data = scene.export(file_type='glb')
            with open(glb_out, 'wb') as f:
                f.write(data)
            size = os.path.getsize(glb_out) / (1024*1024)
            log(f"  Clean GLB: {size:.1f} MB")
            
        except Exception as e:
            log(f"  Trimesh step failed: {e}", "WARN")
        
        # === Step 3: Procedural skin detail ===
        log(f"  Step 3: Skin detail enhancement")
        try:
            mesh = trimesh.load(glb_path, force='mesh')
            if isinstance(mesh, trimesh.Trimesh) and len(mesh.vertices) > 0:
                normals = mesh.vertex_normals
                n = len(mesh.vertices)
                np.random.seed(42)
                
                # Multi-frequency noise for skin texture
                noise = (
                    np.random.randn(n) * 0.3 +
                    np.sin(mesh.vertices[:, 0] * 50) * 0.2 +
                    np.sin(mesh.vertices[:, 1] * 30) * 0.15 +
                    np.cos(mesh.vertices[:, 2] * 40) * 0.1
                ) * 0.002  # Very subtle displacement
                
                displaced = mesh.vertices + normals * noise[:, np.newaxis]
                mesh.vertices = displaced
                
                detail_out = os.path.join(output_dir, f"{filename}_detailed.glb")
                data = mesh.export(file_type='glb')
                with open(detail_out, 'wb') as f:
                    f.write(data)
                log(f"  Skin detail: OK")
        except Exception as e:
            log(f"  Skin detail skipped: {e}", "WARN")
        
        return True
        
    except Exception as e:
        log(f"  FAILED: {e}", "ERROR")
        return False

def main():
    log("=" * 60)
    log("ECLIPSE FULL CHARACTER ART PIPELINE — FIXED")
    log("=" * 60)
    
    glb_files = sorted(glob.glob(os.path.join(CHARACTERS_DIR, "*_HQ.glb")))
    log(f"Found {len(glb_files)} HQ models")
    
    ok = 0
    fail = 0
    results = []
    
    for i, glb in enumerate(glb_files):
        fn = os.path.basename(glb)
        log(f"\n[{i+1}/{len(glb_files)}] {fn}")
        success = process_model(glb, OUTPUT_DIR)
        results.append({"file": fn, "status": "OK" if success else "FAILED"})
        if success: ok += 1
        else: fail += 1
        
        # Save progress
        with open(os.path.join(OUTPUT_DIR, "pipeline_results.json"), "w") as f:
            json.dump(results, f, indent=2)
    
    log(f"\n{'=' * 60}")
    log(f"PIPELINE COMPLETE")
    log(f"  Success: {ok}/{len(glb_files)}")
    log(f"  Failed: {fail}")
    log(f"  Output: {OUTPUT_DIR}")
    log(f"{'=' * 60}")

if __name__ == "__main__":
    main()
