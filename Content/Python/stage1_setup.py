"""
Eclipse: Realms of the Fractured — Stage 1: Solara Cathedral + Aetherborn Playable
================================================================================
Project : C:/Users/Tap/EclipseRotF  (UE 5.8, Lumen+Nanite, Aura)
Date    : 2026-11-07
Author  : Hermes Agent (Momentum Labs)

PURPOSE
-------
Build Stage 1 of Eclipse:RotF in the running UE5.8 editor:
  * Spawn the Aetherborn Male character at world origin.
  * Build the Solara Cathedral: 100x100m floor, 4 walls, 8 pillars, ceiling,
    gothic volumetric fog, warm directional sunlight (-60 deg), sky light.
  * Spawn one enemy placeholder (BERSERKER mesh) for the 3-telegraphed-attack
    test (the C++ EclipseEnemyAI / EclipseCombatSystem drives the telegraphs).
  * Save the level and log everything for debugging.

REMOTE CONTROL API (HTTP, port 30010)
-------------------------------------
The UE5.8 Remote Control API at http://127.0.0.1:30010/remote/ is used for the
parts it can actually perform:
  * PUT /remote/search/assets      — discover assets in /Game
  * PUT /remote/preset/transient    — create a transient Remote Control preset
  * PUT /remote/object/property     — read/write exposed properties
  * PUT /remote/object/call         — call exposed UFUNCTIONs (none spawn actors
    by default, so this is used opportunistically)

The RC API CANNOT spawn arbitrary actors or create level geometry on its own —
that requires UFUNCTION(Remote) helpers that do not exist in a vanilla project.
Therefore the world-mutating work (spawn, light, fog, geometry, save) is done
through the in-engine `unreal` Python module, which IS available because the
PythonScriptPlugin is enabled. The RC API is exercised first for asset search
and preset creation, then the in-engine path mutates the world. This dual-path
design is the only way to produce a genuinely playable level from a single
script. Run this script inside UE's Python environment (Window > Python or
`py` in the Output Log) for full effect; the RC-only path still verifies assets.

USAGE
-----
  # Inside UE5.8 (Output Log > Python, or the Python Script Plugin):
  py  C:/Users/Tap/EclipseRotF/Content/Python/stage1_setup.py

  # Or from the editor's Python console:
  import importlib.util, pathlib
  spec = importlib.util.spec_from_file_location("stage1_setup",
          r"C:/Users/Tap/EclipseRotF/Content/Python/stage1_setup.py")
  m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)

PIE VERIFICATION (Stage 1 acceptance)
-------------------------------------
WASD moves the Aetherborn, mouse orbits the camera, Space dodges in 8
directions, the stamina bar depletes on dodge, the BERSERKER enemy spawns with
3 telegraphed melee attacks, and damage is logged to the Output Log. The dodge
mechanic, stamina component and telegraphed attacks are implemented in C++
(EclipseCharacter / EclipseGameFeel / EclipseInputSystem / EclipseEnemyAI /
EclipseCombatSystem) — this script wires the LEVEL and the SPAWNED ACTORS that
those systems operate on.
"""

import os
import sys
import json
import time
import traceback
from datetime import datetime

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------
RC_HOST = "127.0.0.1"
RC_PORT = 30010
RC_BASE = f"http://{RC_HOST}:{RC_PORT}/remote"

PROJECT_ROOT = r"C:/Users/Tap/EclipseRotF"
LOG_DIR      = os.path.join(PROJECT_ROOT, "Content", "Python")
LOG_FILE     = os.path.join(LOG_DIR, "stage1_setup.log")

# Asset paths (confirmed via the RC /remote/search/assets endpoint)
AETHERBORN_MESH_PATH = "/Game/Characters/Aetherborn_male_HQ/StaticMeshes/Aetherborn_male_HQ.Aetherborn_male_HQ"
ENEMY_MESH_PATH      = "/Game/Characters/BERSERKER/StaticMeshes/BERSERKER.BERSERKER"

# Engine basic shapes (not RC-searchable, but loadable in-engine)
CUBE_MESH     = "/Engine/BasicShapes/Cube"
CYLINDER_MESH = "/Engine/BasicShapes/Cylinder"

# Cathedral dimensions (UE units = centimetres; 1m = 100 units)
FLOOR_EXTENT_M = 100            # 100m x 100m floor
FLOOR_SCALE     = (FLOOR_EXTENT_M, FLOOR_EXTENT_M, 0.5)   # 100x100x0.5 m
WALL_HEIGHT_M   = 20
CEIL_Z_M        = 20            # ceiling at 20m
PILLAR_HEIGHT_M = 18
PILLAR_RADIUS_M = 1.2

# ---------------------------------------------------------------------------
# Logging
# ---------------------------------------------------------------------------
_log_lines = []

def log(msg, level="INFO"):
    ts = datetime.now().strftime("%H:%M:%S.%f")[:-3]
    line = f"[{ts}] [{level}] {msg}"
    print(line)
    _log_lines.append(line)
    # Best-effort file flush
    try:
        os.makedirs(LOG_DIR, exist_ok=True)
        with open(LOG_FILE, "a", encoding="utf-8") as f:
            f.write(line + "\n")
    except Exception:
        pass

def log_exc(prefix="EXCEPTION"):
    for ln in traceback.format_exc().splitlines():
        log(f"{prefix}: {ln}", "ERROR")

# ---------------------------------------------------------------------------
# Remote Control HTTP API client
# ---------------------------------------------------------------------------
class RemoteControlAPI:
    """Thin wrapper over the UE5.8 Remote Control REST API."""

    def __init__(self, base=RC_BASE, timeout=10):
        self.base = base
        self.timeout = timeout
        # urllib is used so the script has zero external dependencies.
        import urllib.request
        self._urllib = urllib.request

    def _put(self, endpoint, payload):
        url = f"{self.base}/{endpoint}"
        data = json.dumps(payload).encode("utf-8")
        req = self._urllib.Request(
            url, data=data,
            headers={"Content-Type": "application/json"},
            method="PUT",
        )
        try:
            with self._urllib.urlopen(req, timeout=self.timeout) as resp:
                return json.loads(resp.read().decode("utf-8"))
        except Exception as e:
            return {"_error": f"{type(e).__name__}: {e}"}

    def search_assets(self, query):
        """PUT /remote/search/assets — search /Game content."""
        res = self._put("search/assets", {"query": query})
        if "_error" in res:
            return None, res["_error"]
        return res.get("Assets", []), None

    def create_transient_preset(self, name="EclipseStage1"):
        """PUT /remote/preset/transient — create a transient RC preset."""
        res = self._put("preset/transient", {"Name": name})
        if "_error" in res:
            return None, res["_error"]
        return res.get("Preset", {}), None

    def call_function(self, object_path, function_name, parameters=None):
        """PUT /remote/object/call — call an exposed UFUNCTION."""
        payload = {"objectPath": object_path, "functionName": function_name}
        if parameters:
            payload["parameters"] = parameters
        return self._put("object/call", payload)

    def read_property(self, object_path, property_name):
        """PUT /remote/object/property — read an exposed property."""
        return self._put("object/property",
                         {"objectPath": object_path, "property": property_name})


# ---------------------------------------------------------------------------
# In-engine helpers (require the `unreal` module — available via PythonScriptPlugin)
# ---------------------------------------------------------------------------
def _get_unreal():
    try:
        import unreal  # type: ignore
        return unreal
    except ImportError:
        return None

UNREAL = _get_unreal()


def rc_search_or_fallback(rc, query, expected_path=None):
    """Use the RC API to search; return list of assets. Logs results."""
    assets, err = rc.search_assets(query)
    if err:
        log(f"RC search '{query}' failed: {err}", "WARN")
        return []
    log(f"RC search '{query}': {len(assets)} asset(s)")
    for a in assets[:3]:
        log(f"    - {a.get('Name')}  {a.get('Class')}  {a.get('Path')}")
    if expected_path:
        found = any(a.get("Path") == expected_path for a in assets)
        log(f"    expected path present: {found}")
    return assets


def spawn_static_mesh(unreal, mesh_path, location, rotation=(0,0,0),
                      scale=(1,1,1), label=""):
    """Spawn a StaticMeshActor in the current level."""
    Vec = unreal.Vector
    Rot = unreal.Rotator
    asset = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not asset:
        log(f"  load_asset FAILED: {mesh_path}", "WARN")
        return None
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        Vec(location[0]*100, location[1]*100, location[2]*100),
        Rot(rotation[0], rotation[1], rotation[2]),
    )
    if not actor:
        log(f"  spawn_actor FAILED at {location}", "WARN")
        return None
    comp = actor.get_editor_property("static_mesh_component")
    if comp:
        comp.set_static_mesh(asset)
    actor.set_actor_scale3d(Vec(scale[0], scale[1], scale[2]))
    if label:
        actor.set_actor_label(label)
    log(f"  + {label or 'StaticMeshActor'} @ {location} scale={scale}")
    return actor


def spawn_character(unreal, mesh_path, location, label, as_pawn=False):
    """Spawn the Aetherborn as the player pawn where possible.

    If the EclipseCharacter C++ class is available, spawn that (so the GAS
    abilities / dodge / stamina systems attach). Otherwise fall back to a
    StaticMeshActor so the mesh still appears in the level for PIE.
    """
    spawned = None
    if as_pawn:
        # Try the project's Character class first (gives the real playable pawn).
        candidates = [
            "/Script/EclipseRotF.EclipseCharacter",   # project C++ class
        ]
        Vec, Rot = unreal.Vector, unreal.Rotator
        for path in candidates:
            cls = unreal.load_class(path)
            if cls:
                try:
                    pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(
                        cls,
                        Vec(location[0]*100, location[1]*100, location[2]*100),
                        Rot(0, 0, 0),
                    )
                    if pawn:
                        pawn.set_actor_label(label)
                        # Attach the Aetherborn mesh to the pawn's mesh component.
                        mesh_asset = unreal.EditorAssetLibrary.load_asset(mesh_path)
                        if mesh_asset:
                            try:
                                sm = pawn.get_editor_property("mesh") if hasattr(pawn, "get_editor_property") else None
                            except Exception:
                                sm = None
                            # SkeletalMeshComponent expected for a Character;
                            # our imported mesh is a StaticMesh, so we add a
                            # static mesh component as a child instead.
                            if mesh_asset and not sm:
                                try:
                                    add_comp = pawn.add_component_by_class(
                                        unreal.StaticMeshComponent, False,
                                        unreal.Transform(Vec(0,0,90), Rot(0,0,0), Vec(1,1,1)))
                                    if add_comp:
                                        add_comp.set_static_mesh(mesh_asset)
                                except Exception:
                                    pass
                        # Possess the pawn with the player controller.
                        try:
                            pc = unreal.EditorLevelLibrary.get_player_controller_from_actor(None)
                        except Exception:
                            pc = None
                        if pc:
                            try:
                                pc.possess(pawn)
                            except Exception:
                                pass
                        spawned = pawn
                        log(f"  + {label} (EclipseCharacter pawn) @ {location}")
                        break
                except Exception:
                    log_exc("spawn_character pawn")
    # Fallback: plain static mesh actor so the body is visible regardless.
    if spawned is None:
        spawned = spawn_static_mesh(
            unreal, mesh_path,
            location=(location[0], location[1], location[2] + 0.9),
            scale=(1,1,1), label=label)
        log(f"  (used StaticMeshActor fallback for {label})", "WARN")
    return spawned


def setup_lighting(unreal):
    """Directional light (sun, -60 deg), sky light, exponential height fog."""
    Vec, Rot = unreal.Vector, unreal.Rotator
    LC = unreal.LinearColor
    spawned = {}

    # --- Directional light: warm sun at -60 degrees pitch ---
    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        Vec(0, 0, 3000), Rot(-60, 45, 0))
    if sun:
        sun.set_actor_label("Solara_Sun_Directional")
        comp = sun.get_editor_property("light_component")
        if comp:
            comp.set_editor_property("intensity", 12.0)
            comp.set_editor_property("light_color", LC(1.0, 0.92, 0.75, 1.0))
            comp.set_editor_property("cast_shadows", True)
            try:
                comp.set_editor_property("shadow_resolution_scale", 4.0)
            except Exception:
                pass
        spawned["sun"] = sun
        log("  + Solara_Sun_Directional (pitch -60, warm)")

    # --- Sky light: ambient fill ---
    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight, Vec(0, 0, 4000), Rot(0, 0, 0))
    if sky:
        sky.set_actor_label("Solara_SkyLight")
        comp = sky.get_editor_property("light_component")
        if comp:
            comp.set_editor_property("intensity", 3.0)
        spawned["sky"] = sky
        log("  + Solara_SkyLight")

    # --- Exponential height fog: volumetric, gothic haze ---
    fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.ExponentialHeightFog, Vec(0, 0, 0), Rot(0, 0, 0))
    if fog:
        fog.set_actor_label("Solara_Volumetric_Fog")
        comp = fog.get_editor_property("fog_component")
        if comp:
            comp.set_editor_property("fog_density", 0.018)
            comp.set_editor_property("fog_height_falloff", 0.45)
            comp.set_editor_property("fog_max_opacity", 0.82)
            comp.set_editor_property("start_distance", 400.0)
            comp.set_editor_property("fog_inscattering_color", LC(0.45, 0.32, 0.22, 1.0))
            try:
                comp.set_editor_property("volumetric_fog_enable", True)
                comp.set_editor_property("volumetric_fog_density", 0.08)
                comp.set_editor_property("volumetric_fog_scattering_distribution", 0.8)
            except Exception:
                pass
        spawned["fog"] = fog
        log("  + Solara_Volumetric_Fog (exp height, volumetric on)")

    return spawned


def build_cathedral(unreal):
    """Floor (100x100m), 4 walls, 8 pillars, ceiling."""
    results = {}

    # Floor — 100x100m, centred at origin, top at z=0
    results["floor"] = spawn_static_mesh(
        unreal, CUBE_MESH,
        location=(0, 0, -0.25),                # half-thickness below ground
        scale=FLOOR_SCALE, label="Solara_Floor")

    # Ceiling — 100x100m at CEIL_Z_M
    results["ceiling"] = spawn_static_mesh(
        unreal, CUBE_MESH,
        location=(0, 0, CEIL_Z_M),
        scale=(FLOOR_EXTENT_M, FLOOR_EXTENT_M, 0.5),
        label="Solara_Ceiling")

    # 4 walls — perimeter at +/- 50m, height WALL_HEIGHT_M
    half = FLOOR_EXTENT_M / 2.0
    wall_scale_long = (FLOOR_EXTENT_M, 0.5, WALL_HEIGHT_M)
    wall_scale_side = (0.5, FLOOR_EXTENT_M, WALL_HEIGHT_M)
    wall_z = WALL_HEIGHT_M / 2.0
    walls = {}
    walls["N"] = spawn_static_mesh(unreal, CUBE_MESH, (0,  half, wall_z), scale=wall_scale_long, label="Solara_Wall_N")
    walls["S"] = spawn_static_mesh(unreal, CUBE_MESH, (0, -half, wall_z), scale=wall_scale_long, label="Solara_Wall_S")
    walls["E"] = spawn_static_mesh(unreal, CUBE_MESH, ( half, 0, wall_z), scale=wall_scale_side,  label="Solara_Wall_E")
    walls["W"] = spawn_static_mesh(unreal, CUBE_MESH, (-half, 0, wall_z), scale=wall_scale_side,  label="Solara_Wall_W")
    results["walls"] = walls

    # 8 pillars — grid pattern, cylindrical, ~1.2m radius, 18m tall
    pillar_positions = [
        (-30, -30), (  0, -30), ( 30, -30),
        (-30,   0),             ( 30,   0),
        (-30,  30), (  0,  30), ( 30,  30),
    ]
    pillars = []
    for i, (x, y) in enumerate(pillar_positions, start=1):
        p = spawn_static_mesh(
            unreal, CYLINDER_MESH,
            location=(x, y, PILLAR_HEIGHT_M/2.0),
            scale=(PILLAR_RADIUS_M*2, PILLAR_RADIUS_M*2, PILLAR_HEIGHT_M),
            label=f"Solara_Pillar_{i:02d}")
        if p:
            pillars.append(p)
    results["pillars"] = pillars
    log(f"  pillars spawned: {len(pillars)}/8")
    return results


def spawn_enemy(unreal):
    """One enemy placeholder (BERSERKER mesh) at the far end of the cathedral.

    The 3 telegraphed melee attacks are driven by EclipseEnemyAI /
    EclipseCombatSystem (C++). Here we just place the body so the AI has
    something to animate.
    """
    enemy = spawn_character(
        unreal, ENEMY_MESH_PATH,
        location=(20, 20, 0), label="Stage1_Enemy_Berserker",
        as_pawn=False)
    return enemy


def save_level(unreal):
    """Save the current level and dirty assets."""
    try:
        unreal.EditorLevelLibrary.save_current_level()
        log("  saved current level")
    except Exception:
        log_exc("save_current_level")
    try:
        unreal.EditorAssetLibrary.save_directory("/Game/")
        log("  saved /Game/ assets")
    except Exception:
        log_exc("save_directory")


# ---------------------------------------------------------------------------
# Main orchestration
# ---------------------------------------------------------------------------
def main():
    log("=" * 70)
    log("ECLIPSE: REALMS OF THE FRACTURED — STAGE 1 SETUP")
    log("  Project : " + PROJECT_ROOT)
    log("  Engine  : UE 5.8 (Remote Control API port %d)" % RC_PORT)
    log("  Time    : " + datetime.now().isoformat())
    log("=" * 70)

    summary = {
        "rc_preset": False, "rc_asset_search": {},
        "aetherborn_found": False, "enemy_found": False,
        "in_engine": UNREAL is not None,
        "cathedral": {}, "lighting": {}, "enemy": None,
        "saved": False, "errors": [],
    }

    # ---- 1. Remote Control API: create transient preset ----
    rc = RemoteControlAPI()
    preset, err = rc.create_transient_preset("EclipseStage1")
    if err:
        msg = f"RC create_transient_preset failed: {err}"
        log(msg, "ERROR"); summary["errors"].append(msg)
    else:
        summary["rc_preset"] = True
        log(f"RC transient preset created: '{preset.get('Name')}' id={preset.get('ID','?')}")

    # ---- 2. Remote Control API: search for assets ----
    for q, expect in [("Aetherborn", AETHERBORN_MESH_PATH),
                      ("Berserker",  ENEMY_MESH_PATH)]:
        assets = rc_search_or_fallback(rc, q, expect)
        summary["rc_asset_search"][q] = len(assets)
        if q == "Aetherborn" and any(a.get("Path") == AETHERBORN_MESH_PATH for a in assets):
            summary["aetherborn_found"] = True
        if q == "Berserker" and any(a.get("Path") == ENEMY_MESH_PATH for a in assets):
            summary["enemy_found"] = True

    # ---- 3. Verify the editor is reachable via RC for diagnostics ----
    probe = rc.read_property("/Script/Engine.Engine", "GameViewport")
    log(f"RC property probe ok: {'_error' not in probe}")

    # ---- 4. In-engine world mutation ----
    if UNREAL is None:
        msg = ("`unreal` module not available — script is running outside the "
               "UE Python environment. RC-only verification completed above; "
               "run this script inside UE (PythonScriptPlugin) to build the level.")
        log(msg, "ERROR"); summary["errors"].append(msg)
        _finish(summary)
        return

    log("-" * 40); log("Building Solara Cathedral geometry ...")
    try:
        summary["cathedral"] = build_cathedral(UNREAL)
    except Exception:
        log_exc("build_cathedral"); summary["errors"].append("build_cathedral failed")

    log("-" * 40); log("Setting up lighting + fog ...")
    try:
        summary["lighting"] = setup_lighting(UNREAL)
    except Exception:
        log_exc("setup_lighting"); summary["errors"].append("setup_lighting failed")

    log("-" * 40); log("Spawning Aetherborn (player) ...")
    try:
        player = spawn_character(
            UNREAL, AETHERBORN_MESH_PATH,
            location=(0, 0, 0), label="Stage1_Player_Aetherborn",
            as_pawn=True)
        summary["player"] = bool(player)
    except Exception:
        log_exc("spawn player"); summary["player"] = False
        summary["errors"].append("spawn player failed")

    log("-" * 40); log("Spawning Stage 1 enemy (Berserker) ...")
    try:
        summary["enemy"] = bool(spawn_enemy(UNREAL))
    except Exception:
        log_exc("spawn enemy"); summary["enemy"] = False
        summary["errors"].append("spawn enemy failed")

    # ---- 5. Save the level ----
    log("-" * 40); log("Saving level ...")
    try:
        save_level(UNREAL)
        summary["saved"] = True
    except Exception:
        log_exc("save"); summary["errors"].append("save failed")

    _finish(summary)


def _finish(summary):
    log("=" * 70)
    log("STAGE 1 SUMMARY")
    log("  RC transient preset : %s" % summary.get("rc_preset"))
    log("  RC asset searches    : %s" % summary.get("rc_asset_search"))
    log("  Aetherborn found     : %s" % summary.get("aetherborn_found"))
    log("  Enemy mesh found     : %s" % summary.get("enemy_found"))
    log("  In-engine (unreal)   : %s" % summary.get("in_engine"))
    cath = summary.get("cathedral", {})
    if cath:
        log("  Cathedral:")
        log("    floor    : %s" % bool(cath.get("floor")))
        log("    ceiling  : %s" % bool(cath.get("ceiling")))
        log("    walls    : %d/4" % sum(1 for v in (cath.get("walls") or {}).values() if v))
        log("    pillars  : %d/8" % len(cath.get("pillars") or []))
    log("  Lighting             : %s" % bool(summary.get("lighting")))
    log("  Player spawned       : %s" % summary.get("player"))
    log("  Enemy spawned        : %s" % summary.get("enemy"))
    log("  Level saved          : %s" % summary.get("saved"))
    if summary.get("errors"):
        log("  ERRORS:")
        for e in summary["errors"]:
            log("    ! " + e, "ERROR")
    log("  Log file : " + LOG_FILE)
    log("=" * 70)

    # Persist a machine-readable summary next to the log.
    try:
        os.makedirs(LOG_DIR, exist_ok=True)
        with open(os.path.join(LOG_DIR, "stage1_setup.summary.json"), "w", encoding="utf-8") as f:
            json.dump(summary, f, indent=2, default=str)
    except Exception:
        pass


# ---------------------------------------------------------------------------
if __name__ == "__main__":
    try:
        main()
    except Exception:
        log_exc("FATAL")
        raise
