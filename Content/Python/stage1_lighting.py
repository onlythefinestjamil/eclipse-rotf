"""
Eclipse Stage 1 Lighting Fix — Use spawn_actor_from_class with loaded classes.
Run inside UE5.8:  py C:/Users/Tap/EclipseRotF/Content/Python/stage1_lighting.py
"""
import os
from datetime import datetime

LOG = os.path.join("C:/Users/Tap/EclipseRotF/Content/Python", "stage1_lighting.log")

def log(msg, level="INFO"):
    line = f"[{datetime.now().strftime('%H:%M:%S.%f')[:-3]}] [{level}] {msg}"
    print(line)
    with open(LOG, "a", encoding="utf-8") as f:
        f.write(line + "\n")

try:
    import unreal
except ImportError:
    log("ERROR: unreal module not available!")
    raise

log("=" * 60)
log("ECLIPSE STAGE 1 — Lighting Fix")
log("=" * 60)

Vec = unreal.Vector
Rot = unreal.Rotator
ELL = unreal.EditorLevelLibrary

# Try different approaches to spawn lighting actors
# Approach 1: Use load_class to get the UE class paths
log("Trying approach 1: spawn_actor_from_class with loaded classes...")

lighting_classes = [
    ("DirectionalLight", "/Script/Engine.DirectionalLight", Vec(0, 0, 3000), Rot(-60, 45, 0), "Solara_Sun"),
    ("SkyLight", "/Script/Engine.SkyLight", Vec(0, 0, 4000), Rot(0, 0, 0), "Solara_SkyLight"),
    ("ExponentialHeightFog", "/Script/Engine.ExponentialHeightFog", Vec(0, 0, 0), Rot(0, 0, 0), "Solara_Fog"),
]

for name, path, loc, rot, label in lighting_classes:
    try:
        cls = unreal.load_class(path)
        if cls:
            actor = ELL.spawn_actor_from_class(cls, loc, rot)
            if actor:
                actor.set_actor_label(label)
                log(f"  + {label} spawned via load_class!")
            else:
                log(f"  ! {name} spawn returned None", "WARN")
        else:
            log(f"  ! {name} load_class returned None", "WARN")
    except Exception as e:
        log(f"  ! {name} approach1 failed: {e}", "ERROR")

# Approach 2: Try using the C++ class directly
log("Trying approach 2: direct class reference...")

try:
    sun = ELL.spawn_actor_from_class(
        unreal.DirectionalLight,
        Vec(0, 0, 3000),
        Rot(-60, 45, 0),
    )
    if sun:
        sun.set_actor_label("Solara_Sun_v2")
        log("  + Sun spawned via direct class!")
except Exception as e:
    log(f"  ! Sun approach2 failed: {e}", "ERROR")

try:
    sky = ELL.spawn_actor_from_class(
        unreal.SkyLight,
        Vec(0, 0, 4000),
        Rot(0, 0, 0),
    )
    if sky:
        sky.set_actor_label("Solara_SkyLight_v2")
        log("  + SkyLight spawned via direct class!")
except Exception as e:
    log(f"  ! SkyLight approach2 failed: {e}", "ERROR")

try:
    fog = ELL.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        Vec(0, 0, 0),
        Rot(0, 0, 0),
    )
    if fog:
        fog.set_actor_label("Solara_Fog_v2")
        log("  + Fog spawned via direct class!")
except Exception as e:
    log(f"  ! Fog approach2 failed: {e}", "ERROR")

# Approach 3: Try placing from content browser actor classes
log("Trying approach 3: Blueprint subclasses...")

# Try to find any lighting actors already in the level
all_actors = ELL.get_all_level_actors()
log(f"  Current actors in level: {len(all_actors)}")
light_types = set()
for a in all_actors:
    try:
        cls_name = a.get_class().get_name()
        light_types.add(cls_name)
    except:
        pass
log(f"  Actor types: {light_types}")

# Save
log("Saving level...")
try:
    ELL.save_current_level()
    log("  + Level saved!")
except Exception as e:
    log(f"  ! Save failed: {e}", "ERROR")

log("=" * 60)
log("LIGHTING FIX COMPLETE")
log("=" * 60)
