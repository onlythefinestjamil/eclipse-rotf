import unreal
import os

log = unreal.log
log("=" * 60)
log("ECLIPSE: Python is working in UE5.8!")
log("=" * 60)

# List all assets in /Game/
assets = unreal.EditorAssetLibrary.list_assets("/Game/", recursive=True)
log(f"Found {len(assets)} assets in /Game/")
for asset in assets[:50]:
    log(f"  {asset}")

# Check if our character files exist
content_dir = unreal.SystemLibrary.get_project_content_directory()
characters_dir = os.path.join(content_dir, "Characters")
glb_files = [f for f in os.listdir(characters_dir) if f.endswith('.glb')] if os.path.exists(characters_dir) else []
log(f"GLB files in Characters: {len(glb_files)}")
for f in glb_files[:10]:
    log(f"  {f}")

log("Python test complete!")
