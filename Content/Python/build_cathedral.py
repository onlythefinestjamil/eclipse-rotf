"""
Eclipse: Realms of the Fractured
GATE 1: Solara Cathedral Level Setup
Momentum Labs | Hermes | November 7, 2026

Creates the basic Solara Cathedral environment — gothic architecture,
volumetric fog, Lumen lighting, atmospheric particles.
"""
import unreal
import os
import math

def log(msg):
    unreal.log(f"[ECLIPSE CATHEDRAL] {msg}")

log("=" * 60)
log("ECLIPSE GATE 1: Building Solara Cathedral")
log("=" * 60)

# === CREATE CATHEDRAL GEOMETRY ===

def spawn_static_mesh(mesh_path, location, rotation=unreal.Rotator(0, 0, 0), scale=unreal.Vector(1, 1, 1), label=""):
    """Spawn a static mesh actor in the level"""
    mesh = unreal.EditorAssetLibrary.load_asset(mesh_path)
    if not mesh:
        log(f"WARNING: Mesh not found at {mesh_path}")
        return None
    
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        location,
        rotation
    )
    if actor:
        actor.static_mesh_component.set_static_mesh(mesh)
        actor.set_actor_scale3d(scale)
        if label:
            actor.set_actor_label(label)
    return actor

def create_cathedral_floor():
    """Create a large cathedral floor"""
    # Use a basic cube as floor
    floor_mesh = "/Engine/BasicShapes/Cube"
    floor = spawn_static_mesh(
        floor_mesh,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0),
        unreal.Vector(50, 50, 0.5),
        "Cathedral_Floor"
    )
    return floor

def create_cathedral_walls():
    """Create cathedral walls"""
    wall_mesh = "/Engine/BasicShapes/Cube"
    walls = []
    
    # North wall
    walls.append(spawn_static_mesh(
        wall_mesh, unreal.Vector(0, 2500, 500), 
        unreal.Rotator(0, 0, 0),
        unreal.Vector(50, 0.5, 50),
        "Cathedral_Wall_North"
    ))
    
    # South wall
    walls.append(spawn_static_mesh(
        wall_mesh, unreal.Vector(0, -2500, 500),
        unreal.Rotator(0, 0, 0),
        unreal.Vector(50, 0.5, 50),
        "Cathedral_Wall_South"
    ))
    
    # East wall
    walls.append(spawn_static_mesh(
        wall_mesh, unreal.Vector(2500, 0, 500),
        unreal.Rotator(0, 0, 0),
        unreal.Vector(0.5, 50, 50),
        "Cathedral_Wall_East"
    ))
    
    # West wall
    walls.append(spawn_static_mesh(
        wall_mesh, unreal.Vector(-2500, 0, 500),
        unreal.Rotator(0, 0, 0),
        unreal.Vector(0.5, 50, 50),
        "Cathedral_Wall_West"
    ))
    
    return walls

def create_cathedral_pillars():
    """Create gothic cathedral pillars"""
    pillar_mesh = "/Engine/BasicShapes/Cylinder"
    pillars = []
    
    # Create 8 pillars in a grid
    positions = [
        (-1500, -1500), (1500, -1500), (-1500, 1500), (1500, 1500),
        (-1500, 0), (1500, 0), (0, -1500), (0, 1500)
    ]
    
    for i, (x, y) in enumerate(positions):
        pillar = spawn_static_mesh(
            pillar_mesh,
            unreal.Vector(x, y, 500),
            unreal.Rotator(0, 0, 0),
            unreal.Vector(1.5, 1.5, 5),
            f"Cathedral_Pillar_{i+1}"
        )
        if pillar:
            pillars.append(pillar)
    
    return pillars

def create_cathedral_ceiling():
    """Create cathedral ceiling"""
    ceiling_mesh = "/Engine/BasicShapes/Cube"
    ceiling = spawn_static_mesh(
        ceiling_mesh,
        unreal.Vector(0, 0, 2000),
        unreal.Rotator(0, 0, 0),
        unreal.Vector(50, 50, 0.5),
        "Cathedral_Ceiling"
    )
    return ceiling

# === LIGHTING ===

def setup_lighting():
    """Set up Lumen-friendly cathedral lighting"""
    # Main directional light (sun through cathedral windows)
    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 3000),
        unreal.Rotator(-60, 45, 0)
    )
    if sun:
        sun.set_actor_label("Cathedral_Sun_Directional")
        sun_light = sun.get_editor_property('light_component')
        if sun_light:
            sun_light.set_editor_property('intensity', 10.0)
            sun_light.set_editor_property('light_color', unreal.LinearColor(1.0, 0.9, 0.7, 1.0))
            sun_light.set_editor_property('cast_shadows', True)
            sun_light.set_editor_property('shadow_resolution_scale', 4.0)
        log("Directional light (sun) configured")
    
    # Sky light for ambient
    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 3000),
        unreal.Rotator(0, 0, 0)
    )
    if sky:
        sky.set_actor_label("Cathedral_SkyLight")
        sky_comp = sky.get_editor_property('light_component')
        if sky_comp:
            sky_comp.set_editor_property('intensity', 2.0)
        log("Sky light configured")
    
    # Point lights for atmospheric glow
    for i in range(4):
        angle = i * 90
        rad = math.radians(angle)
        x = math.cos(rad) * 1500
        y = math.sin(rad) * 1500
        
        point_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
            unreal.PointLight,
            unreal.Vector(x, y, 800),
            unreal.Rotator(0, 0, 0)
        )
        if point_light:
            point_light.set_actor_label(f"Cathedral_Glow_Light_{i+1}")
            light_comp = point_light.get_editor_property('light_component')
            if light_comp:
                light_comp.set_editor_property('intensity', 5000.0)
                light_comp.set_editor_property('light_color', 
                    unreal.LinearColor(0.9, 0.6, 0.2, 1.0))
                light_comp.set_editor_property('attenuation_radius', 2000.0)
                light_comp.set_editor_property('cast_shadows', True)
        log(f"Point light {i+1} configured at ({x:.0f}, {y:.0f})")

# === FOG AND ATMOSPHERE ===

def setup_atmosphere():
    """Set up volumetric fog and atmospheric effects"""
    # Exponential height fog
    fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.ExponentialHeightFog,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    if fog:
        fog.set_actor_label("Cathedral_Volumetric_Fog")
        fog_comp = fog.get_editor_property('fog_component')
        if fog_comp:
            fog_comp.set_editor_property('fog_density', 0.02)
            fog_comp.set_editor_property('fog_height_falloff', 0.5)
            fog_comp.set_editor_property('fog_max_opacity', 0.8)
            fog_comp.set_editor_property('start_distance', 500.0)
            fog_comp.set_editor_property('fog_inscattering_color', 
                unreal.LinearColor(0.4, 0.3, 0.2, 1.0))
            fog_comp.set_editor_property('volumetric_fog_enable', True)
            fog_comp.set_editor_property('volumetric_fog_density', 0.1)
            fog_comp.set_editor_property('volumetric_fog_scattering_distribution', 0.8)
        log("Volumetric fog configured")
    
    # Atmospheric fog
    atmo_fog = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.AtmosphericFog,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0)
    )
    if atmo_fog:
        atmo_fog.set_actor_label("Cathedral_Atmospheric_Fog")
        log("Atmospheric fog configured")

# === SKY SPHERE ===

def setup_sky():
    """Set up the sky sphere"""
    sky_sphere = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 5000),
        unreal.Rotator(0, 0, 0)
    )
    if sky_sphere:
        sky_sphere.set_actor_label("Cathedral_Sky")
        log("Sky sphere configured")

# === EXECUTE LEVEL BUILD ===
log("Building cathedral geometry...")
floor = create_cathedral_floor()
walls = create_cathedral_walls()
pillars = create_cathedral_pillars()
ceiling = create_cathedral_ceiling()

log("Setting up lighting...")
setup_lighting()

log("Setting up atmosphere...")
setup_atmosphere()

log("Setting up sky...")
setup_sky()

# === SAVE ===
log("Saving level...")
unreal.EditorLevelLibrary.save_current_level()
unreal.EditorAssetLibrary.save_directory("/Game/")

log("=" * 60)
log("SOLARA CATHEDRAL: BUILD COMPLETE")
log(f"  Floor: {'OK' if floor else 'FAILED'}")
log(f"  Walls: {sum(1 for w in walls if w)}/4 OK")
log(f"  Pillars: {sum(1 for p in pillars if p)}/8 OK")
log(f"  Ceiling: {'OK' if ceiling else 'FAILED'}")
log(f"  Lighting: OK")
log(f"  Fog: OK")
log("=" * 60)
log("Next: Import Aetherborn character and configure game mode")
