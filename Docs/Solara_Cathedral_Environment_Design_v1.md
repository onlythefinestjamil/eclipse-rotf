# SOLARA CATHEDRAL — ENVIRONMENT DESIGN DOCUMENT
## Eclipse: Realms of the Fractured — UE5.8
### World Director | Momentum Labs | November 8, 2026 (v2 — corrected)
### STATUS: DESIGN COMPLETE + VERIFIED IN-ENGINE STATE — screenshots exist but show broken placeholder, not a cathedral

---

## 1. CONCEPT OVERVIEW

**Region:** Solara — Sun-scorched cathedral lands (Fire + Holy aligned)
**Location:** Cathedral of Unmade Hours — the Day 1 vertical slice target
**Role:** First playable environment, 300m hero path from vista to boss arena

The Cathedral of Unmade Hours is a colossal Gothic cathedral caught in a temporal collapse. The roof is ripped open, revealing a cosmic void. Floating brass clocks, gears, and broken stone fragments hang suspended in mid-air. Purple crackling energy veins pulse through the architecture. Stained glass windows glow with ethereal cold blue and violet light. Warm directional sunlight pierces through the breach in the ceiling.

**Art Direction (V4 LOCKED):** Ancient over ornamental. Weathered, hand-forged, tool marks, oxidation. ONE restrained supernatural element per area. Not WoW item-icon aesthetic.

---

## 2. ARCHITECTURE SPECIFICATION

### 2.1 Structural Layout (300m playable path)

```
ZONE 1: Vista Point (0m)
├── High overlook on cliff edge
├── Panoramic view of sun-scorched desert + cathedral exterior
├── Warm amber directional light, golden hour
└── Player spawns here, gets first look at the cathedral

ZONE 2: Cathedral Approach (50m)
├── Weathered stone pathway with cracked tiles
├── Collapsed outer walls — half-buried in sand
├── Ancient bronze doors with sun deity carvings
├── Heat haze shimmer, floating ash particles
└── Environmental storytelling: abandoned offerings, broken statues

ZONE 3: Cathedral Nave — Interior (100m)
├── Central aisle flanked by massive stone pillars
├── Pointed Gothic arches soaring 30m+ vertically
├── Stained glass windows (intact lower, shattered upper)
├── Ribbed vault ceiling partially collapsed — cosmic void visible
├── Floating debris: clocks, gears, stone fragments frozen in time
├── Purple energy veins crackling through architecture
└── Volumetric god rays from ceiling breach

ZONE 4: Grace Site Checkpoint (180m)
├── Side chapel with intact vault
├── Bonfire-style save point (Grace Site)
├── Warm amber lighting — safe zone contrast
├── Rest area with environmental storytelling (campfire, bedroll)
└── Transition to boss arena corridor

ZONE 5: Boss Arena — Cathedral Apse (200m)
├── Raised altar platform at far end
├── Destructible stone pillars (for Pyre-Warden Velkor fight)
├── Collapsed floor sections revealing lava rivers below
├── Colossal stained glass rose window behind altar
└── Boss: Pyre-Warden Velkor
    ├── Phase 1 (100-70%): Sword combos, frontal cleave
    ├── Phase 2 (70-35%): Destructible pillars fall, arena shrinks
    ├── Phase 3 (35-0%): Time manipulation — rewinds player dodge
    └── Enrage timer: 5 minutes
```

### 2.2 Architectural Elements

**Gothic Features Required:**
- Pointed arches (lancet, trefoil, quatrefoil)
- Ribbed vault ceilings (partially collapsed)
- Flying buttresses (exterior)
- Stained glass windows (rose + lancet)
- Stone tracery with intricate religious carvings
- Gargoyles and grotesques on exterior
- Column statues (saints/kings, weathered and headless)
- Ribbed piers with attached shafts

**Material Palette:**
- Dark weathered basalt stone (primary)
- Aged sandstone (warm accents, sun-facing surfaces)
- Brass/bronze (clockwork elements, door fittings)
- Stained glass (blue, violet, amber panes)
- Moss/lichen on shaded stone surfaces
- Cracked mortar with visible structural damage

**Scale:** Human feels insignificant. Pillars 4-6m diameter. Arches 15-25m tall. Nave ceiling 30-40m. Total interior length ~200m.

---

## 3. NANITE GEOMETRY PLAN

### 3.1 Nanite-Enabled Assets

All architectural geometry uses Nanite for fine detail without performance cost:

| Asset | Nanite Usage | Detail Level |
|-------|------------|--------------|
| Stone pillars | Carvings, cracks, weathering | 8M+ tris source |
| Archways | Tracery, moldings, stone texture | 4M+ tris source |
| Stained glass frames | Stone tracery, leading | 2M+ tris source |
| Gargoyle statues | Full sculptural detail | 6M+ tris source |
| Altar platform | Religious bas-relief carvings | 5M+ tris source |
| Floor tiles | Cracks, wear patterns, moss | 3M+ tris source |
| Collapsed ceiling edges | Broken stone, rebar | 4M+ tris source |

### 3.2 Nanite Configuration
- Enable Nanite on all static mesh assets
- Set NaniteProxyRenderPercent: 0.8 (80% screen error → proxy)
- Position error threshold: 0.3 (default, good for RX 6800)
- Material complexity: 1-2 material layers per asset (keep shader cost low)
- Virtual Shadow Maps (VSM) enabled for all Nanite geometry (already in config)

### 3.3 Non-Nanite (Skeletal/Animated)
- Floating debris (clocks, gears) — skeletal mesh with animation
- Destructible pillars — geometry collection (Chaos destruction)
- Stained glass shatter — fracture-based destruction

---

## 4. LUMEN LIGHTING CONFIGURATION

### 4.1 Post-Process & GI Settings

**Current Config (DefaultEngine.ini — already set):**
```
r.Lumen.GlobalIllumination=True
r.Lumen.Reflections=True
r.Lumen.ScreenSpaceGI=True
r.Lumen.ScreenSpaceReflection=True
r.LumenRayLightingMode=2
r.LumenSceneLightingQuality=2
r.Lumen.DiffuseIndirectLightingAllow=1
r.LumenRayLighting=True
r.Lumen.DirectLighting=True
r.LumenSurfaceCacheRefreshPeriod=1
r.Shadow.Virtual.Enable=1
r.VolumetricFog=True
```

**Solara Cathedral-Specific Lighting Plan:**

| Element | Setting | Value |
|---------|---------|-------|
| Sun (Directional Light) | Intensity | 10 lux (warm amber) |
| Sun | Color | (0.9, 0.7, 0.3) warm amber |
| Sun | Angle | 28° elevation, piercing through ceiling breach |
| Sky Light | Intensity | 3 (cool fill from cosmic void) |
| Sky Light | Color | (0.3, 0.2, 0.5) dim violet |
| Post-Process | Exposure | Auto (min 0.3, max 2.0) |
| Post-Process | Bloom | Intensity 1.2 (stained glass glow) |
| Post-Process | Vignette | 0.6 (dark corners for cathedral) |
| Post-Process | Contrast | 1.1 (dramatic chiaroscuro) |
| Post-Process | Saturation | 0.85 (slightly desaturated, ancient) |
| Post-Process | Tonemapper | ACES (filmic, high dynamic range) |

### 4.2 Volumetric Fog Settings
- Volumetric fog density: 0.3 (configurable via EclipseSolaraLevel::FogDensity)
- God rays from ceiling breach: ExponentialHeightFog with direction aligned to sun
- Fog color: warm amber near sun, cool violet in shadows
- Particle-driven fog: local volume actors for purple haze in nave
- Fog particle count: 2,000 max (RX 6800 budget)

### 4.3 Lumen Performance Tuning for RX 6800
- r.Lumen.ScreenSpaceGI=True (cheaper fallback for GI)
- r.Lumen.HardwareRayTracing=False (RX 6800 = RDNA2, no DXR acceleration — use software Lumen)
- r.Lumen.ReflectionsType=1 (screen-space reflections, not ray-traced)
- r.Shadow.Virtual.Enable=1 (VSM for all shadows — already set)
- r.LumenRayLightingMode=2 (highest quality software mode)
- Target: 60 FPS at 1080p, Lumen software mode, 2M total visible triangles

---

## 5. QUIXEL MEGASCANS INTEGRATION

### 5.1 Plugin Status
- **Plugin EXISTS:** C:/Program Files/Epic Games/UE_5.8/Engine/Plugins/MegascansPlugin/
- **ENABLED in EclipseRotF.uproject** as of World Director run Nov 8 (was previously NOT enabled)
- **Takes effect on next UE5.8 editor launch** (requires editor restart)
- **No assets downloaded** yet — MSPresets folder only

### 5.2 Required Megascans Assets for Solara Cathedral

**Architectural Surfaces (3D):**
- Gothic stone pillar assets (weathered, cracked)
- Stone archway segments
- Ribbed vault ceiling sections
- Stone floor tiles (cracked, moss-covered)
- Gargoyle statues
- Stone railing sections

**Surface Materials (2D):**
- Weathered basalt stone (dark, cracked)
- Aged sandstone (warm, sun-bleached)
- Moss-covered stone
- Cracked mortar
- Stained glass material (emissive, refractive)
- Brass/bronze (tarnished, aged)
- Sand/sandstone (exterior)

**Foliage/Debris (3D):**
- Dead/dried vines
- Rubble piles (stone fragments)
- Dust/debris particles
- Sand drifts (exterior)

### 5.3 Asset Storage Plan
**CRITICAL:** C: drive is FULL (16GB free). All Megascans assets MUST be stored on H: (405GB free) or E: (925GB free).

**Recommended approach:**
1. Create DerivedDataCache on H: (redirect in DefaultEngine.ini)
2. Download Megascans assets to H:/EclipseRotF_Megascans/
3. Use UE5.8's redirector to mount external content directory
4. Or use Project Content mount via .uproject additional directories

### 5.4 Quixel Bridge Workflow
1. Enable MegascansPlugin in .uproject (add to Plugins array)
2. Launch Quixel Bridge from UE5.8 editor (Window → Quixel Bridge)
3. Login with Epic Games account (free with UE5)
4. Browse → search "gothic", "cathedral", "stone", "ruins"
5. Download assets to H: drive
6. Import into Content/Environments/Solara/ subfolders
7. Apply Nanite to all imported static meshes
8. Configure material instances for Lumen compatibility

---

## 6. WORLD PARTITION PLAN

### 6.1 Current State
- World Partition NOT configured in project
- No DataLayer setup
- No level streaming config

### 6.2 Solara Cathedral World Partition Design

Since the cathedral is a 300m linear path (not open world), World Partition is optional but recommended for:
- Streaming the exterior approach separately from interior
- Keeping boss arena geometry separate for destruction prep
- Enabling future expansion to full Solara region

**Recommended:**
- Convert Lvl_SolaraCathedral to World Partition level
- Data Layers: 
  - DL_Exterior (cathedral approach, 0-50m)
  - DL_Nave (main interior, 50-180m)
  - DL_BossArena (altar area, 180-250m)
  - DL_CosmicVoid (ceiling breach VFX, always loaded)
- Cell size: 25,600 units (~128m) — 2-3 cells total
- Loading: distance-based, 1-cell radius

---

## 7. ENVIRONMENTAL STORYTELLING

### 7.1 Narrative Elements

| Location | Element | Story Told |
|----------|---------|-----------|
| Approach | Abandoned offerings at doors | People worshipped here, fled suddenly |
| Nave entrance | Broken pews, scattered prayer books | Congregation fled during the temporal event |
| Mid-nave | Frozen figures in stone (petrified worshippers) | The collapse petrified those inside |
| Side chapels | Crude campsites, bedrolls | Survivors sheltered here after the event |
| Grace Site | Intact shrine with eternal flame | Hope persists even in ruin |
| Near altar | Broken weapons, scorch marks | The Pyre-Warden has fought intruders before |
| Altar | Massive shattered clock face | Time itself broke here — the "Unmade Hours" |
| Floor cracks | Purple energy veins | The Fracture's corruption spreads through stone |

### 7.2 Audio-Visual Storytelling
- Distant bell tolls (muffled, echoing through nave)
- Stone settling/groaning sounds (structural stress)
- Wind through ceiling breach (howling, ambient)
- Ember crackling near fire hazards
- Energy vein crackling (intermittent, threatening)

---

## 8. PERFORMANCE BUDGET — RX 6800 16GB

### 8.1 GPU Budget (60 FPS @ 1080p)

| Category | Budget | Notes |
|----------|--------|-------|
| Nanite rendering | 8ms | 2M visible triangles, software VSM |
| Lumen GI | 6ms | Software Lumen (no HW RT on RDNA2) |
| Volumetric fog | 3ms | 2K particle max, exponential height fog |
| Shadows (VSM) | 3ms | Virtual shadow maps, 4096 max res |
| Post-processing | 2ms | ACES tonemapper, bloom, vignette |
| Translucency | 2ms | Stained glass, energy particles |
| Overhead | 3ms | Draw calls, instancing, culling |
| **Total** | **27ms** | **~37 FPS minimum, target 16.6ms for 60 FPS** |

### 8.2 Optimization Concerns

1. **Lumen on RDNA2 (RX 6800):** No hardware DXR. Software Lumen uses screen-space + SDF tracing. Quality is good but expensive. Consider r.Lumen.ScreenSpaceGI=True for cheaper fallback.

2. **Nanite + VSM:** Virtual Shadow Maps are the recommended shadow solution for Nanite geometry. Already enabled. Watch for VSM page thrashing on complex scenes.

3. **Volumetric fog density:** 0.3 is a reasonable starting point. If FPS drops, reduce to 0.15 or use local fog volumes instead of global.

4. **Stained glass translucency:** Each pane is a translucent material. Limit visible panes to 8-12 at once. Use forward-rendered translucency for glass.

5. **Particle systems:** Floating debris (clocks, gears) should be limited to ~50 visible at once. Energy veins should be GPU-simulated, not CPU.

6. **VRAM:** 16GB is sufficient for 1080p with Lumen + Nanite. Watch texture streaming pool — set to 8-10GB to avoid hitches.

7. **C: drive full (16GB):** DerivedDataCache and intermediate builds MUST go to H: or E:. Set in DefaultEngine.ini:
   ```
   [SystemSettings]
   r.DerivedDataCache.Path=H:/EclipseRotF_DDC
   ```

### 8.3 Fallback Quality Settings (if 60 FPS not achieved)

| Level | Setting | Impact |
|-------|---------|--------|
| High | Lumen software, 1080p, 2M tris | Target baseline |
| Medium | SSR + SSAO, 1080p, 1.5M tris | Disable Lumen GI |
| Low | Baked lighting, 900p, 1M tris | Emergency fallback |

---

## 9. IMPLEMENTATION ROADMAP

### Phase 1: Foundation (REQUIRES UE5.8 EDITOR — CANNOT DO FROM CRON)
1. ~~Enable MegascansPlugin in .uproject~~ ✅ DONE (World Director, Nov 8)
2. Set DDC path to H: in DefaultEngine.ini (NOT DONE — C: still 17GB free, H: has 405GB)
3. Download Gothic/cathedral Megascans assets via Quixel Bridge (needs interactive Epic login)
4. Import to Content/Environments/Solara/
5. Enable Nanite on all imported meshes

### Phase 2: Blockout & Lighting
1. Block out cathedral geometry in Lvl_SolaraCathedral
2. Set up Directional Light (warm amber, 28° angle)
3. Set up Sky Light (cool violet fill)
4. Configure ExponentialHeightFog for god rays
5. Add volumetric particle systems for purple haze
6. Place Post-Process Volume with ACES tonemapper

### Phase 3: Dressing & Storytelling
1. Replace blockout with Megascans assets
2. Add environmental storytelling props
3. Create material instances for Lumen compatibility
4. Add stained glass emissive materials
5. Place Grace Site checkpoint
6. Set up Chaos destruction for boss arena pillars

### Phase 4: Optimization & Validation
1. GPU Visualizer profiling (stat gpu)
2. Nanite visualization (Nanite stats)
3. Lumen visualization (Lumen stats)
4. Test at 1080p 60 FPS target
5. Adjust fog density, particle counts, Lumen quality
6. Capture in-engine screenshots
7. Report verified FPS numbers

### Phase 5: World Partition & Expansion
1. Convert to World Partition level
2. Set up Data Layers (Exterior, Nave, BossArena, CosmicVoid)
3. Test streaming behavior
4. Plan exterior approach zone
5. Plan full Solara region connection points

---

## 10. CURRENT ASSET INVENTORY

| Item | Status | Location |
|------|--------|----------|
| Lvl_SolaraCathedral.umap | EXISTS (55KB — likely placeholder) | Content/Maps/ |
| EclipseSolaraLevel.cpp/h | EXISTS (C++ level controller) | Source/EclipseRotF/ |
| Lumen/Nanite/VFog config | ENABLED in DefaultEngine.ini | Config/ |
| MegascansPlugin | INSTALLED but NOT enabled | UE5.8 Engine/Plugins/ |
| Megascans assets | NOT downloaded | — |
| Cathedral concept art | 2 images (interior) | HANDOFF_FORGE_ENVIRONMENT/ |
| Gothic City concept art | 3 images (exterior) | HANDOFF_FORGE_ENVIRONMENT/ |
| Environment prompts (12) | COMPLETE | HANDOFF_FORGE_ENVIRONMENT/ |
| World Map concept | 1 image | HANDOFF_FORGE_ENVIRONMENT/ |
| Content/Environments/ | EMPTY | No assets imported |
| Content/Materials/ | EMPTY | No custom materials |
| World Partition | NOT configured | — |

---

## 11. HONEST STATUS (v2 — CORRECTED, verified Nov 8)

### 11.1 What ACTUALLY exists in-engine (VERIFIED from screenshots)

Malene (Windows agent) ran a series of Stage 1 Python scripts in the UE5.8 editor on Nov 7 (13:27–15:05 EST), importing a cathedral GLB and capturing **12 in-engine screenshots** to `C:/Users/Tap/Desktop/`:

| Screenshot | Time | Size | What it shows |
|-----------|------|------|---------------|
| Stage1_Cathedral_Screenshot.png | 13:27 | 86KB | Initial import — near-black |
| Stage1_Cathedral_LightingFixed.png | 13:35 | 92KB | After first lighting pass |
| Stage1_Final_Lighting.png | 13:36 | 102KB | Directional + SkyLight + fog added |
| Stage1_VISIBLE.png | 13:40 | 1.1MB | Boosted lights, sky sphere added |
| Stage1_TonedDown.png | 13:42 | 2.1MB | Toned back — still ~99% black, heavy orange cast |
| Stage1_AAA_Replace.png | 14:56 | 927KB | Swapped to AAA cathedral mesh |
| Stage1_Cathedral_AAA_Imported.png | 14:58 | 937KB | AAA mesh imported |
| Stage1_Cathedral_AAA_v2.png | 15:00 | 935KB | v2 import (Interchange + GLTF both failed, mesh not spawned) |
| Stage1_Cathedral_Environment.png | 15:05 | 1.0MB | Final — cathedral placed as environment |

**Verified assessment (World Director, vision-reviewed Nov 8):**
- **Stage1_Cathedral_Environment.png (final):** Does NOT show a cathedral. Shows a vertical stack of dark box-like meshes (resembling a ziggurat/crate stack, not Gothic architecture) rendered as pure black silhouettes against an orange sky. No ground plane, no pillars, no arches, no stained glass, no fog, no shadows. Floating stray geometry. Missing materials (everything unlit black). Heavy orange tint.
- **Stage1_TonedDown.png:** Severely underexposed — ~99% of scene is pure black. Flat floor split into two placeholder test materials. Tiny silhouetted statue on pedestal. No visible Gothic architecture. No fog. Broken lighting (no Lumen GI visible).
- **Root cause:** The cathedral GLB (SOLARA_CATHEDRAL_AAA.glb, 28MB) imported as a stack of unlit boxes, not a Gothic cathedral. The Interchange pipeline and GLTF import both failed (v2 log: "Interchange failed", "GLTFImportOptions not found", "Could not spawn cathedral"). Materials are unassigned/black. Lighting is broken (underexposed, orange cast, no Lumen GI functioning).

### 11.2 What exists (design + config)

- ✅ UE5.8 project with correct render settings (Lumen, Nanite, VSM, volumetric fog in DefaultEngine.ini)
- ✅ C++ level controller (EclipseSolaraLevel.cpp/h) defining 5 zones — UPDATED to V9 canon (Pyre-Warden Velkor boss, Nov 8)
- ✅ MegascansPlugin ENABLED in .uproject (World Director, Nov 8 — takes effect on next editor launch)
- ✅ Comprehensive environment design doc (this file) — architecture, lighting, Nanite, performance budgets
- ✅ 12 detailed environment concept prompts + 5 cathedral/Gothic concept images in HANDOFF_FORGE_ENVIRONMENT/
- ✅ Cathedral GLB imported (SOLARA_CATHEDRAL_AAA.glb, 28MB) but imports as broken geometry

### 11.3 What does NOT exist

- ❌ NO functional cathedral environment — imported mesh is a broken stack of unlit boxes, not Gothic architecture
- ❌ NO Megascans assets downloaded or placed (plugin now enabled but Quixel Bridge not logged in)
- ❌ NO working Lumen GI in the scene (screenshots show unlit/underexposed geometry)
- ❌ NO volumetric fog visible in any screenshot
- ❌ NO Nanite-enabled Gothic geometry (no stone pillars, arches, tracery, stained glass)
- ❌ NO World Partition configured
- ❌ NO validated 60 FPS performance numbers
- ❌ NO Quixel Bridge authenticated (needs interactive Epic Games login)
- ❌ NO materials created (Content/Materials/ is empty)
- ❌ NO destructible pillar setup for boss arena (Chaos Geometry Collection)
- ❌ NO DDC redirect to H: (C: still 17GB free — at risk of filling)
- ❌ NO environmental storytelling props placed
