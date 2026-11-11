# SOLARA CATHEDRAL — ENVIRONMENT DESIGN DOCUMENT v3
## Eclipse: Realms of the Fractured — UE5.8
### World Director | Momentum Labs | November 11, 2026
### STATUS: DESIGN COMPLETE — NOT BUILT IN-ENGINE (verified: no cathedral geometry, no screenshots showing Gothic architecture)

---

## 0. HONEST CURRENT STATE (Nov 11, 2026)

### What EXISTS:
- ✅ UE5.8 project with correct render config (Lumen software, Nanite, VSM, volumetric fog in DefaultEngine.ini)
- ✅ MegascansPlugin enabled in .uproject (takes effect on editor restart)
- ✅ C++ level controller (EclipseSolaraLevel.cpp/h) — 5 zones, Pyre-Warden Velkor boss (stub logging only)
- ✅ solara.py lighting setup script (Directional Light, Sky Light, ExponentialHeightFog, PostProcessVolume) — NEVER SUCCESSFULLY RUN (no v2 screenshot exists on Desktop)
- ✅ Lvl_SolaraCathedral.umap (55KB — placeholder, no Gothic geometry)
- ✅ SOLARA_CATHEDRAL_AAA mesh imported (672K Nanite tris) — but imports as broken stack of unlit boxes, NOT a cathedral
- ✅ DDC redirected to K:/Eclipse_DDC/ (junction for Intermediate active)
- ✅ This design document (architecture, lighting, Nanite, performance, Megascans plan)
- ✅ 12 environment concept prompts + 5 cathedral/Gothic concept images in HANDOFF_FORGE_ENVIRONMENT/

### What DOES NOT EXIST:
- ❌ NO functional Gothic cathedral environment — verified by vision analysis of latest screenshot (Nov 7): shows broken placeholder pillar silhouette against orange void, NO Gothic architecture, NO fog, NO Lumen GI, NO stained glass
- ❌ NO Megascans assets downloaded or placed (Quixel Bridge needs interactive Epic login)
- ❌ NO working Lumen GI visible in any screenshot (all show flat/unlit geometry)
- ❌ NO volumetric fog visible in any screenshot
- ❌ NO Nanite-enabled Gothic geometry (pillars, arches, tracery, stained glass)
- ❌ NO World Partition configured
- ❌ NO validated 60 FPS performance numbers (no profiling done)
- ❌ NO materials created (Content/Materials/ is EMPTY)
- ❌ NO destructible pillar setup for boss arena (Chaos Geometry Collection)
- ❌ NO environmental storytelling props placed
- ❌ solara.py lighting script never successfully executed

### CRITICAL ISSUE: C: drive at 100% (2.1GB free)
All new assets, Megascans downloads, and DDC MUST go to K: (3.2TB free). UE5.8 editor may fail to launch or compile if C: fills completely.

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

### 2.2 Gothic Architectural Elements Required

- Pointed arches (lancet, trefoil, quatrefoil)
- Ribbed vault ceilings (partially collapsed)
- Flying buttresses (exterior)
- Stained glass windows (rose + lancet)
- Stone tracery with intricate religious carvings
- Gargoyles and grotesques on exterior
- Column statues (saints/kings, weathered and headless)
- Ribbed piers with attached shafts

### 2.3 Material Palette
- Dark weathered basalt stone (primary)
- Aged sandstone (warm accents, sun-facing surfaces)
- Brass/bronze (clockwork elements, door fittings)
- Stained glass (blue, violet, amber panes)
- Moss/lichen on shaded stone surfaces
- Cracked mortar with visible structural damage

### 2.4 Scale
Human feels insignificant. Pillars 4-6m diameter. Arches 15-25m tall. Nave ceiling 30-40m. Total interior length ~200m.

---

## 3. NANITE GEOMETRY PLAN

### 3.1 Nanite-Enabled Assets (All Megascans Source)

| Asset | Megascans Search Term | Nanite Tris Budget | Detail Level |
|-------|----------------------|--------------------|-------------|
| Stone pillars (x8 pairs) | "gothic pillar", "stone column" | 500K each = 4M total | Carvings, cracks, weathering |
| Archway segments (x12) | "stone arch", "gothic arch" | 300K each = 3.6M total | Tracery, moldings |
| Ribbed vault sections (x6) | "vault ceiling", "ribbed vault" | 400K each = 2.4M total | Structural detail |
| Stained glass frames (x8) | "stone window frame", "gothic tracery" | 250K each = 2M total | Stone tracery, leading |
| Gargoyle statues (x4) | "gargoyle", "stone statue" | 600K each = 2.4M total | Full sculptural detail |
| Altar platform | "stone altar", "church altar" | 800K total | Bas-relief carvings |
| Floor tiles | "stone floor", "cathedral floor" | 1M total (tiled) | Cracks, wear, moss |
| Collapsed ceiling edges | "broken stone", "rubble" | 500K total | Broken stone, rebar |
| Exterior walls | "stone wall", "castle wall" | 1.5M total | Weathered, cracked |
| Flying buttresses (x4) | "flying buttress", "stone support" | 400K each = 1.6M total | Structural Gothic |

**TOTAL VISIBLE NANITE BUDGET: ~2M triangles on-screen at once** (RX 6800 limit)
- Source assets: ~20M tris total, Nanite culls to 2M visible per frame
- NaniteProxyRenderPercent: 0.8
- Position error threshold: 0.3

### 3.2 Non-Nanite (Skeletal/Animated)
- Floating debris (clocks, gears) — skeletal mesh with animation, ~50 max visible
- Destructible pillars — Chaos Geometry Collection (boss arena)
- Stained glass shatter — fracture-based destruction

---

## 4. LUMEN LIGHTING CONFIGURATION

### 4.1 Render Settings (DefaultEngine.ini — ALREADY SET)

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
r.Nanite.Enabled=True
r.Nanite.Programmable=True
```

### 4.2 Solara Cathedral Lighting Plan

| Element | Setting | Value | Purpose |
|---------|---------|-------|---------|
| Directional Light (Sun) | Intensity | 10 lux | Warm amber god rays through ceiling breach |
| Sun | Color | (0.9, 0.7, 0.3) | Golden hour warmth |
| Sun | Rotation | (-42, 0, 28) | 28° elevation, pierces breach |
| Sun | Temperature | 5500K | Warm daylight |
| Sky Light | Intensity | 3.0 | Cool fill from cosmic void above |
| Sky Light | Color | (0.3, 0.2, 0.5) | Dim violet — void glow |
| Sky Light | Lower Hemisphere | (0.05, 0.03, 0.08) | Very dark — contrast |
| ExponentialHeightFog | Density | 0.015 | Visible but not opaque |
| Fog | Color | (0.15, 0.1, 0.05) | Warm amber near sun |
| Fog | Volumetric | Enabled | God rays from ceiling breach |
| Fog | Scattering Distribution | 0.8 | Strong directional scattering |
| Fog | Extinction Scale | 8.0 | Dense god ray effect |
| Fog | Distance | 6000 units | Localized fog volume |
| Post-Process | Bloom | 1.2 | Stained glass glow |
| Post-Process | Vignette | 0.6 | Dark corners — cathedral mood |
| Post-Process | Contrast | 1.1 | Dramatic chiaroscuro |
| Post-Process | Saturation | 0.85 | Slightly desaturated, ancient |
| Post-Process | Auto Exposure | min 0.3, max 2.0 | HDR adaptation |
| Post-Process | Tonemapper | ACES | Filmic high dynamic range |
| Post-Process | Film Grain | 0.15 | Subtle texture |
| Post-Process | Chromatic Aberration | 0.1 | Subtle lens imperfection |

### 4.3 Lumen Performance Tuning for RX 6800 (RDNA2 — NO hardware DXR)

**CRITICAL:** RX 6800 is RDNA2 with NO hardware ray tracing acceleration. All Lumen uses software mode (screen-space + SDF tracing).

Console commands to apply in-editor (or add to DefaultEngine.ini [SystemSettings]):
```
r.Lumen.HardwareRayTracing=False
r.Lumen.ReflectionsType=1
r.Lumen.ScreenSpaceGI=True
r.Lumen.Scan.RadianceProbes=False
r.Lumen.TranslucencyVolumeResolution=8
r.Lumen.ScreenSpaceGather.SampleCount=4
r.Lumen.ScreenSpaceGather.DownsampleFactor=2
r.Lumen.DiffuseIndirectAllow=1
r.Shadow.Virtual.Enable=1
r.Shadow.Virtual.MaxPhysicalPages=4096
r.Shadow.TileResolution=8
r.VolumetricFog.GridSize=8
r.VolumetricFog.HistoryWeight=0.9
r.VolumetricFog.InjectShadowDensity=1
```

---

## 5. VOLUMETRIC FOG DESIGN

### 5.1 God Ray System
- Primary: ExponentialHeightFog with volumetric enabled — sun direction creates god rays through ceiling breach
- Density: 0.015 (start conservative, increase to 0.03 if too subtle)
- Scattering distribution: 0.8 (strong forward scattering for dramatic rays)
- Extinction scale: 8.0 (concentrates god ray effect near breach)

### 5.2 Local Fog Volumes
- Purple haze in nave interior — local ExponentialHeightFog volume, density 0.02
- Ash/dust particles near floor — Niagara particle system, 500 particles max
- Heat haze at approach zone — distortion material on a plane, no particles

### 5.3 Fog Particle Budget
- Global volumetric fog: 0 cost (built into Lumen/volumetric system)
- Local Niagara particle systems: 2,000 particles TOTAL max (RX 6800 budget)
- God rays: free (volumetric fog + directional light interaction)

---

## 6. QUIXEL MEGASCANS ASSET SHOPPING LIST

### 6.1 Plugin Status
- MegascansPlugin: ENABLED in .uproject (takes effect on editor restart)
- Quixel Bridge: NOT AUTHENTICATED (needs interactive Epic Games login in UE5.8 editor)
- Assets download location: K:/Eclipse_Data/Megascans/ (create before downloading)

### 6.2 Required Megascans Assets (Search Terms for Quixel Bridge)

**3D Architectural Assets (Priority 1 — structure):**
| # | Search Term | Type | Qty | Purpose |
|---|------------|------|-----|---------|
| 1 | "gothic pillar" | 3D | 16 | Nave columns (8 pairs) |
| 2 | "stone arch" | 3D | 12 | Gothic archways |
| 3 | "ribbed vault" | 3D | 6 | Ceiling vault sections |
| 4 | "stone window frame" | 3D | 8 | Stained glass frames |
| 5 | "gargoyle statue" | 3D | 4 | Exterior detail |
| 6 | "stone altar" | 3D | 1 | Boss arena altar |
| 7 | "flying buttress" | 3D | 4 | Exterior support |
| 8 | "stone wall ruined" | 3D | 10 | Collapsed exterior walls |
| 9 | "cobblestone" / "stone floor" | 3D | 4 | Floor tiles |
| 10 | "rubble pile" / "debris" | 3D | 6 | Collapse storytelling |

**Surface Materials (Priority 2 — surfaces):**
| # | Search Term | Type | Purpose |
|---|------------|------|---------|
| 11 | "weathered stone" | Material | Primary basalt surfaces |
| 12 | "aged sandstone" | Material | Warm sun-facing accents |
| 13 | "moss stone" | Material | Shaded surfaces |
| 14 | "cracked mortar" | Material | Structural damage |
| 15 | "brass tarnished" | Material | Clockwork/door fittings |
| 16 | "sand desert" | Material | Exterior ground |

**Foliage/Debris (Priority 3 — detail):**
| # | Search Term | Type | Purpose |
|---|------------|------|---------|
| 17 | "dead vine" | 3D | Wall climbing decay |
| 18 | "dried plant" | 3D | Floor debris |
| 19 | "rock formation" | 3D | Exterior landscape |
| 20 | "dead tree" | 3D | Vista point framing |

### 6.3 Quixel Bridge Workflow (REQUIRES EDITOR + EPIC LOGIN)
1. Launch UE5.8 editor (MUST restart for Megascans plugin activation)
2. Window → Quixel Bridge
3. Login with Epic Games account (free with UE5)
4. Set download path to K:/Eclipse_Data/Megascans/
5. Search each asset above, download, import to Content/Environments/Solara/
6. Enable Nanite on all imported static meshes
7. Configure material instances for Lumen compatibility (roughness, metallic, emissive for glass)

---

## 7. WORLD PARTITION PLAN

### 7.1 Current State
- World Partition: NOT configured
- No DataLayer setup
- No level streaming config

### 7.2 Design (for future expansion — optional for 300m linear path)
- Convert Lvl_SolaraCathedral to World Partition level
- Data Layers:
  - DL_Exterior (cathedral approach, 0-50m)
  - DL_Nave (main interior, 50-180m)
  - DL_BossArena (altar area, 180-250m)
  - DL_CosmicVoid (ceiling breach VFX, always loaded)
- Cell size: 25,600 units (~128m) — 2-3 cells total
- Loading: distance-based, 1-cell radius

---

## 8. ENVIRONMENTAL STORYTELLING

| Location | Element | Story Told |
|----------|---------|------------|
| Approach | Abandoned offerings at doors | People worshipped here, fled suddenly |
| Nave entrance | Broken pews, scattered prayer books | Congregation fled during temporal event |
| Mid-nave | Frozen figures in stone (petrified worshippers) | The collapse petrified those inside |
| Side chapels | Crude campsites, bedrolls | Survivors sheltered here after the event |
| Grace Site | Intact shrine with eternal flame | Hope persists even in ruin |
| Near altar | Broken weapons, scorch marks | The Pyre-Warden has fought intruders before |
| Altar | Massive shattered clock face | Time itself broke here — the "Unmade Hours" |
| Floor cracks | Purple energy veins | The Fracture's corruption spreads through stone |

**Audio-Visual:**
- Distant bell tolls (muffled, echoing through nave)
- Stone settling/groaning sounds (structural stress)
- Wind through ceiling breach (howling, ambient)
- Ember crackling near fire hazards
- Energy vein crackling (intermittent, threatening)

---

## 9. PERFORMANCE BUDGET — RX 6800 16GB — REVISED FOR 60 FPS

### 9.1 THE PROBLEM WITH v2 BUDGET
v2 budget estimated 27ms total = ~37 FPS. This is BELOW the 60 FPS target (16.6ms).
v3 budget is reworked to fit in 16.6ms.

### 9.2 REVISED GPU Budget (60 FPS @ 1080p = 16.6ms)

| Category | v2 Budget | v3 Budget | Optimization Applied |
|----------|-----------|-----------|---------------------|
| Nanite rendering | 8ms | 5.0ms | Cap visible tris at 1.5M (not 2M), aggressive NaniteProxyRenderPercent=0.7 |
| Lumen GI (software) | 6ms | 5.0ms | ScreenSpaceGI=True, DownsampleFactor=2, reduced probe count |
| Volumetric fog | 3ms | 1.5ms | GridSize=8 (not 16), reduce extinction scale to 6.0, local volumes only |
| Shadows (VSM) | 3ms | 2.0ms | MaxPhysicalPages=4096, TileResolution=8, cap shadow res at 2048 |
| Post-processing | 2ms | 1.5ms | Reduce bloom quality to 3, motion blur off, DOF quality 2 |
| Translucency | 2ms | 1.0ms | Limit visible glass panes to 6-8, forward-rendered translucency |
| Overhead | 3ms | 0.6ms | Instancing for repeated assets (pillars, tiles), aggressive culling |
| **TOTAL** | **27ms (37 FPS)** | **16.6ms (60 FPS)** | **Hits target** |

### 9.3 Console Commands for 60 FPS Target (apply in editor or DefaultEngine.ini)

```
[SystemSettings]
; --- Nanite optimization ---
r.Nanite.MaxVisibleTriangles=1500000
r.NaniteProxyRenderPercent=0.7
r.Nanite.PositionErrorThreshold=0.3
r.Nanite.PixelsPerTriangle=2.0

; --- Lumen software optimization (RDNA2, no DXR) ---
r.Lumen.HardwareRayTracing=False
r.Lumen.ScreenSpaceGI=True
r.Lumen.ScreenSpaceGather.DownsampleFactor=2
r.Lumen.ScreenSpaceGather.SampleCount=4
r.Lumen.TranslucencyVolumeResolution=8
r.Lumen.Scan.RadianceProbes=False
r.Lumen.DiffuseIndirectAllow=1
r.LumenSceneLightingQuality=2

; --- Shadow optimization ---
r.Shadow.Virtual.Enable=1
r.Shadow.Virtual.MaxPhysicalPages=4096
r.Shadow.TileResolution=8
r.Shadow.MaxResolution=2048
r.Shadow.RadiusThreshold=0.04

; --- Volumetric fog optimization ---
r.VolumetricFog.GridSize=8
r.VolumetricFog.HistoryWeight=0.9
r.VolumetricFog.HistoryMissCount=5
r.VolumetricFog.InjectShadowDensity=1

; --- Post-process optimization ---
r.BloomQuality=3
r.MotionBlurQuality=0
r.DepthOfFieldQuality=2
r.LensFlareQuality=2
r.EyeAdaptationQuality=1
r.ToneShaper=2
r.Tonemapper.Sharpen=1

; --- Texture streaming ---
r.Streaming.PoolSize=8192
r.Streaming.LimitPoolSizeToVRAM=True
r.Streaming.MaxEffectiveScreenSize=1080
```

### 9.4 VRAM Budget (16GB RX 6800 @ 1080p)
| Category | Allocation |
|----------|-----------|
| Texture streaming pool | 8GB |
| Lumen surface cache | 1GB |
| Nanite meshes (visible) | 2GB |
| VSM shadow pages | 0.5GB |
| Render targets + GBuffer | 2GB |
| OS + driver overhead | 2.5GB |
| **Total** | **16GB** |

### 9.5 Fallback Quality Settings (if 60 FPS not achieved)

| Level | Setting | Expected FPS | Impact |
|-------|---------|-------------|--------|
| High (target) | Lumen software, 1080p, 1.5M tris | 60 FPS | Target baseline |
| Medium | SSR + SSAO, 1080p, 1M tris | 60 FPS | Disable Lumen GI, use baked AO |
| Low | Baked lighting, 900p, 800K tris | 60 FPS | Emergency fallback, no dynamic GI |

### 9.6 Performance Concerns (HONEST)

1. **Lumen software on RDNA2 is the biggest risk.** No hardware DXR means all GI is screen-space + SDF traced. Quality is acceptable but the 5ms budget is aggressive. If Lumen eats 7ms+, we drop to ~50 FPS and need to fall back to Medium quality (SSR + SSAO).

2. **Volumetric fog + god rays are expensive.** GridSize=8 (instead of 16) helps, but the ceiling breach god ray effect is central to the cathedral's identity. If fog costs >2ms, reduce extinction scale to 4.0 or use local fog volumes only.

3. **Stained glass translucency** is a hidden cost. Each pane is a translucent material with refraction. Limit to 6-8 visible panes at once. Use forward-rendered translucency for glass. If >8 panes visible, FPS will drop.

4. **Nanite triangle budget is tight.** 1.5M visible (not 2M) because Lumen needs GPU time too. The cathedral has ~20M source tris — Nanite culling must reduce this to 1.5M per frame. This works if the player can't see all pillars/arches simultaneously (line-of-sight occlusion in the nave).

5. **VSM page thrashing** on complex Gothic geometry (intricate carvings, tracery) can cause shadow flicker or performance spikes. MaxPhysicalPages=4096 should handle it, but watch for shadow quality drops.

6. **No profiling has been done.** All budgets are estimates. Real GPU Visualizer (stat gpu) profiling is needed once the cathedral is built. Until then, these are targets, not verified numbers.

---

## 10. IMPLEMENTATION ROADMAP

### Phase 1: Foundation (REQUIRES UE5.8 EDITOR — CANNOT DO FROM CRON)
**BLOCKER: C: drive 100% full (2.1GB free). Editor may not launch.**
1. ~~Enable MegascansPlugin in .uproject~~ ✅ DONE
2. ~~Set DDC path to K: in DefaultEngine.ini~~ ✅ DONE (junction active)
3. Create K:/Eclipse_Data/Megascans/ directory
4. Restart UE5.8 editor (Megascans plugin activation)
5. Login Quixel Bridge with Epic account (interactive — needs user)
6. Download Gothic/cathedral Megascans assets (Section 6.2 list) to K:
7. Import to Content/Environments/Solara/
8. Enable Nanite on all imported meshes

### Phase 2: Blockout & Lighting
1. Run solara.py in editor console (py content/python/solara.py) — sets up lights + fog + post-process
2. Block out cathedral geometry with Megascans assets (pillars, arches, vaults)
3. Verify Lumen GI is functioning (check for light bounces, indirect lighting)
4. Capture screenshot — verify Gothic architecture is visible
5. Adjust sun angle, fog density, post-process for mood

### Phase 3: Dressing & Storytelling
1. Replace blockout with final Megascans asset placement
2. Add environmental storytelling props (Section 8)
3. Create stained glass emissive materials (blue/violet/amber panes)
4. Place Grace Site checkpoint (bonfire-style)
5. Set up Chaos destruction for boss arena pillars
6. Add floating debris (clocks, gears) — skeletal mesh animation

### Phase 4: Optimization & Validation
1. Run `stat gpu` — capture GPU frame time breakdown
2. Run `stat unit` — verify <16.6ms total
3. Apply console commands from Section 9.3
4. Test at 1080p 60 FPS target
5. Adjust fog density, particle counts, Lumen quality as needed
6. Capture in-engine screenshots showing FINISHED cathedral
7. Report verified FPS numbers (not estimates)

### Phase 5: World Partition & Expansion
1. Convert to World Partition level
2. Set up Data Layers (Exterior, Nave, BossArena, CosmicVoid)
3. Test streaming behavior
4. Plan exterior approach zone
5. Plan full Solara region connection points

---

## 11. NEXT ACTION (for Jamil)

**Immediate blockers requiring YOUR action (cannot do from cron/CLI):**
1. **C: drive is 100% full (2.1GB free).** UE5.8 may not launch. Need to clear space or the editor won't compile/run.
2. **Quixel Bridge needs your Epic Games login** in the UE5.8 editor (Window → Quixel Bridge → Sign In).
3. **solara.py needs to be run manually** in the editor console: `py content/python/solara.py`

**Once you're in the editor:**
1. Restart UE5.8 (Megascans plugin activation)
2. Login Quixel Bridge
3. Download assets from Section 6.2 shopping list
4. Run solara.py for lighting setup
5. I'll handle the blockout and dressing plan from there
