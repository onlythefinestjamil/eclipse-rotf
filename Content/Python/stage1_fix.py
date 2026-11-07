"""
Eclipse Stage 1 Fix — Add lighting + player spawn to the already-built Cathedral.
Run inside UE5.8:  py C:/Users/Tap/EclipseRotF/Content/Python/stage1_fix.py
"""
import os, json, time
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_fix.log")

def log(msg, level="INFO"):
    line = f"[{datetime.now().strftime('%H:%M:%S.%f')[:-3]}] [{level}] {msg}"
    print(line)
    with open(LOG, "a", encoding="utf-8") as f:
        f.write(line + "\n")

try:
    import unreal
except ImportError:
    log("ERROR: unreal module not available - run inside UE5.8!")
    raise

log("=" * 60)
log("ECLIPSE STAGE 1 FIX — Lighting + Player Spawn")
log("=" * 60)

Vec = unreal.Vector
Rot = unreal.Rotator
ELL = unreal.EditorLevelLibrary
EAL = unreal.EditorAssetLibrary

# === LIGHTING ===
log("Setting up lighting...")

# Directional Light (sun)
try:
    sun = ELL.spawn_actor_from_object(
        unreal.DirectionalLight,
        Vec(0, 0, 3000),
        Rot(-60, 45, 0),
    )
    if sun:
        sun.set_actor_label("Solara_Sun")
        log(f"  + Sun spawned: {sun.get_name()}")
    else:
        log("  ! Sun spawn returned None", "WARN")
except Exception as e:
    log(f"  ! Sun failed: {e}", "ERROR")

# Sky Light
try:
    sky = ELL.spawn_actor_from_object(
        unreal.SkyLight,
        Vec(0, 0, 4000),
        Rot(0, 0, 0),
    )
    if sky:
        sky.set_actor_label("Solara_SkyLight")
        log(f"  + SkyLight spawned: {sky.get_name()}")
except Exception as e:
    log(f"  ! SkyLight failed: {e}", "ERROR")

# Exponential Height Fog
try:
    fog = ELL.spawn_actor_from_object(
        unreal.ExponentialHeightFog,
        Vec(0, 0, 0),
        Rot(0, 0, 0),
    )
    if fog:
        fog.set_actor_label("Solara_Fog")
        log(f"  + Fog spawned: {fog.get_name()}")
except Exception as e:
    log(f"  ! Fog failed: {e}", "ERROR")

# === PLAYER (Aetherborn as StaticMeshActor) ===
log("Spawning Aetherborn player...")
AETHERBORN = "/Game/Characters/Aetherborn_male_HQ/StaticMeshes/Aetherborn_male_HQ.Aetherborn_male_HQ"

try:
    mesh_asset = EAL.load_asset(AETHERBORN)
    if mesh_asset:
        player = ELL.spawn_actor_from_class(
            unreal.StaticMeshActor,
            Vec(0, 0, 90),
            Rot(0, 0, 0),
        )
        if player:
            player.set_actor_label("Stage1_Player_Aetherborn")
            comp = player.get_editor_property("static_mesh_component")
            if comp:
                comp.set_static_mesh(mesh_asset)
            log(f"  + Aetherborn player spawned at origin")
            log(f"  + Mesh: {AETHERBORN}")
        else:
            log("  ! Player spawn returned None", "ERROR")
    else:
        log(f"  ! Could not load Aetherborn mesh asset", "ERROR")
except Exception as e:
    log(f"  ! Player spawn failed: {e}", "ERROR")

# === SAVE ===
log("Saving level...")
try:
    ELL.save_current_level()
    log("  + Level saved!")
except Exception as e:
    log(f"  ! Save failed: {e}", "ERROR")

log("=" * 60)
log("STAGE 1 FIX COMPLETE")
log("=" * 60)
