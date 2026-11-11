# SOLARA CATHEDRAL — ENVIRONMENT DESIGN DOCUMENT v2
## Eclipse: Realms of the Fractured — UE5.8
### World Director | Momentum Labs | November 9, 2026
### STATUS: DESIGN COMPLETE | MESH EXISTS (672K Nanite tris) | NO MATERIALS, LIGHTING, FOG, OR MEGASCANS DRESSING

---

## 0. VERIFIED PROJECT STATE (Nov 9, Remote Control API + asset search)

### What EXISTS (verified via API):
- SOLARA_CATHEDRAL_AAA StaticMesh — Nanite enabled, 672,282 Nanite triangles, 380,200 Nanite vertices, ~111x17x191 units
- AETHERBORN_HERO_AAA StaticMesh — Nanite, 4.36M Nanite tris (character model, not environment)
- BERSERKER StaticMesh — Nanite, 2.44M Nanite tris (enemy, not environment)
- Lvl_SolaraCathedral.umap (55KB — likely empty/placeholder)
- Lvl_Stage1_Arena.umap (20KB — active combat test level)
- EclipseSolaraLevel.cpp/h — C++ level controller with 5 zone definitions
- Render config in DefaultEngine.ini: Lumen GI, Nanite, VSM, volumetric fog ALL enabled
- MegascansPlugin — EXISTS in engine plugins, NOW ENABLED in .uproject (takes effect on next restart)
- Remote Control API running on port 30010 (asset search confirmed working)

### What DOES NOT EXIST:
- ❌ NO proper materials on cathedral mesh (using default Interchange opaque, no textures)
- ❌ NO lighting actors in Lvl_SolaraCathedral (no Directional Light, Sky Light, fog, or post-process)
- ❌ NO Megascans assets downloaded or placed (Quixel Bridge needs interactive Epic login)
- ❌ NO volumetric fog in the scene
- ❌ NO environmental storytelling props
- ❌ NO World Partition or Data Layers
- ❌ NO destructible pillars for boss arena (Chaos Geometry Collection)
- ❌ NO validated 60 FPS performance numbers
- ❌ NO in-engine screenshots showing a finished cathedral (existing screenshots show broken/placeholder)

---

## 1. CONCEPT OVERVIEW

**Region:** Solara — Sun-scorched cathedral lands (Fire + Holy aligned)
**Location:** Cathedral of Unmade Hours — Day 1 vertical slice target
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
├── Destructible stone pillars (Chaos Geometry Collection)
├── Collapsed floor sections revealing lava rivers below
├── Colossal stained glass rose window behind altar
└── Boss: Pyre-Warden Velkor
    ├── Phase 1 (100-70%): Sword combos, frontal cleave
    ├── Phase 2 (70-35%): Destructible pillars fall, arena shrinks
    ├── Phase 3 (35-0%): Time manipulation — rewinds player dodge
    └── Enrage timer: 5 minutes
```

### 2.2 Gothic Features Required
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

**Scale:** Human feels insignificant. Pillars 4-6m diameter. Arches 15-25m tall. Nave ceiling 30-40m. Total interior length ~200m.

---

## 3. NANITE GEOMETRY PLAN

### 3.1 Existing Nanite Asset
| Asset | Nanite Tris | Nanite Verts | Status |
|-------|------------|-------------|--------|
| SOLARA_CATHEDRAL_AAA | 672,282 | 380,200 | Exists, Nanite ON, NO materials |

### 3.2 Required Additional Nanite Assets (Megascans)
| Asset Category | Megascans Search Terms | Nanite Budget |
|---------------|----------------------|--------------|
| Gothic stone pillars | "gothic pillar", "stone column ruins" | 4-8M tris source each |
| Stone archways | "stone arch", "gothic archway" | 2-4M tris source |
| Ribbed vault sections | "vault ceiling", "stone ceiling ruins" | 4M+ tris source |
| Stone floor tiles | "cracked stone floor", "cathedral floor" | 2M+ tris source |
| Gargoyle statues | "gargoyle", "stone statue" | 3-6M tris source |
| Stone rubble/debris | "rubble", "stone debris", "ruins" | 1-2M tris source |
| Sand/exterior ground | "sand", "desert ground", "arid terrain" | 2M tris source |

### 3.3 Nanite Configuration
- Enable Nanite on ALL imported Megascans static meshes
- Position error threshold: 0.3 (default, good for RX 6800)
- NaniteProxyRenderPercent: 0.8
- Material complexity: 1-2 layers per asset (keep shader cost low)
- VSM enabled for all Nanite geometry (already in DefaultEngine.ini)

### 3.4 Non-Nanite (Skeletal/Animated)
- Floating debris (clocks, gears) — skeletal mesh with animation
- Destructible pillars — Chaos Geometry Collection
- Stained glass shatter — fracture-based destruction

---

## 4. LUMEN LIGHTING CONFIGURATION

### 4.1 Engine-Level Config (DefaultEngine.ini — VERIFIED SET)
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
r.AntiAliasingMethod=3          # TAA
r.TemporalAA.Upsampling=True
r.BloomQuality=5
r.LensFlareQuality=3
r.EyeAdaptationQuality=2
r.ToneShaper=2
r.Tonemapper.Sharpen=1
r.Tonemapper.GrainQuantization=True
r.Shadow.MaxResolution=4096
r.ShadowQuality=4
r.MotionBlurQuality=4
r.DepthOfFieldQuality=4
```

### 4.2 Scene-Level Lighting (to be set in Lvl_SolaraCathedral)

| Actor | Property | Value | Notes |
|-------|----------|-------|-------|
| Directional Light (Sun) | Intensity | 10 lux | Warm amber directional |
| Directional Light | Light Color | (0.9, 0.7, 0.3) | Warm amber RGB |
| Directional Light | Temperature | 5500K | Warm daylight |
| Directional Light | Rotation | Pitch -42, Yaw 0, Roll 28 | 28° elevation, pierces ceiling breach |
| Directional Light | Cast Shadows | True | VSM shadows |
| Sky Light | Intensity | 3.0 | Cool fill from cosmic void |
| Sky Light | Light Color | (0.3, 0.2, 0.5) | Dim violet |
| Sky Light | Lower Hemisphere | (0.05, 0.03, 0.08) | Very dark, dramatic contrast |
| ExponentialHeightFog | Fog Density | 0.015 | Moderate, not overpowering |
| ExponentialHeightFog | Fog Color | (0.15, 0.1, 0.05) | Warm amber tint |
| ExponentialHeightFog | Volumetric Fog | Enabled | God rays through ceiling breach |
| ExponentialHeightFog | Scattering Distribution | 0.8 | Forward-scattering for god rays |
| ExponentialHeightFog | Extinction Scale | 8.0 | Controls fog thickness |
| PostProcess Volume | Bloom Intensity | 1.2 | Stained glass glow |
| PostProcess Volume | Vignette | 0.6 | Dark corners for cathedral |
| PostProcess Volume | Contrast | 1.1 | Dramatic chiaroscuro |
| PostProcess Volume | Saturation | 0.85 | Slightly desaturated, ancient |
| PostProcess Volume | Auto Exposure Min | 0.3 | Prevent total darkness |
| PostProcess Volume | Auto Exposure Max | 2.0 | Allow bright god rays |
| PostProcess Volume | Film Grain | 0.15 | Subtle texture |
| PostProcess Volume | Unbound | True | Affects entire level |

### 4.3 Lumen Performance Tuning for RX 6800 (RDNA2, NO DXR)
- r.Lumen.HardwareRayTracing=False (RX 6800 = RDNA2, no DXR)
- r.Lumen.ScreenSpaceGI=True (cheaper GI fallback)
- r.Lumen.ReflectionsType=1 (screen-space reflections, not ray-traced)
- r.LumenRayLightingMode=2 (highest quality software mode)
- Software Lumen uses screen-space + SDF tracing — no hardware ray tracing

---

## 5. VOLUMETRIC FOG & ATMOSPHERIC DESIGN

### 5.1 Fog Layers
1. **Global ExponentialHeightFog** — base atmosphere, warm amber, 0.015 density
   - Volumetric fog enabled for god rays
   - Height falloff: 0.4 (fogs concentrated lower in nave)
   - Start distance: 200 units (fog starts close to camera)

2. **Local fog volumes** (per-zone Niagara particle systems)
   - Zone 3 (Nave): Purple energy haze — local particle volume, 500 particles max
   - Zone 2 (Approach): Heat shimmer + ash particles — 200 particles
   - Zone 5 (Boss Arena): Ember/smoke near lava — 300 particles

3. **God rays** — ExponentialHeightFog aligned to sun direction through ceiling breach
   - Scattering distribution 0.8 (strong forward scatter)
   - Extinction scale 8.0 (visible shafts)

### 5.2 Particle Budget (RX 6800)
| System | Particles | Type | Notes |
|--------|-----------|------|-------|
| Global volumetric fog | 0 (uses ExpHeightFog) | Built-in | No particle cost |
| Purple energy haze | 500 max | GPU Niagara | Local volume in nave |
| Floating ash | 200 max | GPU Niagara | Approach zone |
| Ember/smoke | 300 max | GPU Niagara | Boss arena |
| Floating debris (clocks/gears) | 50 max | Skeletal | Animated, not particles |
| **Total particle budget** | **1,000 max** | | Well under 2K limit |

---

## 6. QUIXEL MEGASCANS INTEGRATION

### 6.1 Plugin Status
- MegascansPlugin EXISTS in: C:/Program Files/Epic Games/UE_5.8/Engine/Plugins/MegascansPlugin/
- NOW ENABLED in EclipseRotF.uproject (World Director, Nov 9) — takes effect on next UE5.8 restart
- 20 Megascans preset materials available (MS_DefaultMaterial, Metal_MasterMaterial, Foliage_Material, etc.)
- No Megascans assets downloaded yet — Quixel Bridge needs interactive Epic Games login

### 6.2 Required Megascans Assets for Solara Cathedral

**Architectural 3D Assets (download via Quixel Bridge):**
| Asset | Search Terms | Use Zone | Priority |
|-------|-------------|----------|----------|
| Gothic stone pillars | "gothic pillar", "stone column" | Nave, Boss Arena | HIGH |
| Stone archway sections | "stone arch", "gothic archway" | Nave | HIGH |
| Ribbed vault ceiling | "vault ceiling", "ribbed vault" | Nave | MEDIUM |
| Cracked stone floor | "cathedral floor", "stone tiles" | All interior | HIGH |
| Gargoyle statues | "gargoyle", "stone grotesque" | Exterior | LOW |
| Stone rubble | "rubble pile", "stone debris" | All | MEDIUM |
| Collapsed wall sections | "ruined wall", "broken wall stone" | Approach | MEDIUM |
| Sand/desert ground | "sand", "arid ground", "desert" | Vista, Approach | HIGH |
| Dead vegetation | "dead vine", "dried plant" | Approach, Nave | LOW |
| Bronze/brass props | "metal fitting", "bronze fixture" | Doors, altar | LOW |

**Surface Materials (2D Megascans textures):**
| Material | Search Terms | Apply To |
|----------|-------------|----------|
| Weathered basalt | "basalt", "dark stone weathered" | Cathedral walls |
| Aged sandstone | "sandstone", "warm stone" | Sun-facing surfaces |
| Moss-covered stone | "moss stone", "lichen rock" | Shaded stone |
| Cracked mortar | "cracked plaster", "mortar" | Wall joints |
| Stained glass | (custom material) | Windows — emissive + refractive |
| Tarnished brass | "bronze", "aged metal" | Clockwork, fittings |
| Sand | "sand", "desert sand" | Exterior ground |

### 6.3 Asset Storage Plan
**CRITICAL:** C: drive is 99% full (~23GB free). ALL Megascans downloads MUST go to K:/Eclipse_Data/ (3.2TB free).

1. DDC already redirected to K:/Eclipse_DDC/ via DefaultEngine.ini
2. Intermediate/Saved already junctioned to K:/Eclipse_Data/EclipseRotF_Backup/
3. Download Megascans to K:/Eclipse_Data/Megascans/
4. Import into Content/Environments/Solara/ subfolders
5. Enable Nanite on all imported static meshes

### 6.4 Quixel Bridge Workflow (REQUIRES EDITOR GUI — CANNOT DO FROM CRON)
1. Restart UE5.8 (for MegascansPlugin to activate)
2. Open Window → Quixel Bridge in UE5.8 editor
3. Login with Epic Games account (free with UE5)
4. Search "gothic", "cathedral", "stone ruins", "pillar"
5. Download assets to K: drive (set download path in Bridge settings)
6. Import into Content/Environments/Solara/
7. Enable Nanite on all imported static meshes
8. Apply material instances for Lumen compatibility
9. Save all imported assets

---

## 7. WORLD PARTITION PLAN

### 7.1 Current State
- World Partition NOT configured
- No DataLayer setup
- No level streaming config

### 7.2 Solara Cathedral World Partition Design
Since the cathedral is a 300m linear path (not open world), World Partition is optional but recommended for:
- Streaming exterior approach separately from interior
- Keeping boss arena geometry separate for destruction prep
- Enabling future expansion to full Solara region

**Recommended:**
- Convert Lvl_SolaraCathedral to World Partition level
- Data Layers: DL_Exterior (0-50m), DL_Nave (50-180m), DL_BossArena (180-250m), DL_CosmicVoid (ceiling VFX, always loaded)
- Cell size: 25,600 units (~128m) — 2-3 cells total
- Loading: distance-based, 1-cell radius

---

## 8. ENVIRONMENTAL STORYTELLING

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

**Audio-Visual Storytelling:**
- Distant bell tolls (muffled, echoing through nave)
- Stone settling/groaning sounds (structural stress)
- Wind through ceiling breach (howling, ambient)
- Ember crackling near fire hazards
- Energy vein crackling (intermittent, threatening)

---

## 9. PERFORMANCE BUDGET — RX 6800 16GB (RDNA2, NO DXR)

### 9.1 GPU Budget (60 FPS @ 1080p = 16.6ms frame)

| Category | Budget | Notes |
|----------|--------|-------|
| Nanite rendering | 5ms | 672K tris cathedral + Megascans dressing (~2M total visible) |
| Lumen GI (software) | 5ms | Software SDF tracing, no hardware DXR on RDNA2 |
| Volumetric fog | 2ms | ExpHeightFog + 1K local particles |
| Shadows (VSM) | 2ms | Virtual shadow maps, 4096 max res |
| Post-processing | 2ms | ACES tonemapper, bloom, vignette, TAA |
| Translucency | 1ms | Stained glass, energy particles |
| Overhead | 1.6ms | Draw calls, instancing, culling |
| **Total** | **18.6ms** | **~54 FPS estimated; tighten to hit 60** |

### 9.2 Optimization Concerns

1. **Lumen on RDNA2 (RX 6800):** No hardware DXR. Software Lumen uses screen-space + SDF tracing. Quality is good but ~30% more expensive than hardware Lumen. The 5ms budget is tight — if FPS drops, reduce LumenSceneLightingQuality to 1 and LumenRayLightingMode to 1.

2. **Nanite + VSM:** Virtual Shadow Maps are the recommended shadow solution for Nanite geometry. Already enabled. Watch for VSM page thrashing if too many shadow-casting lights are added.

3. **Volumetric fog cost:** God rays through the ceiling breach are the most expensive fog element. If FPS drops, reduce extinction scale from 8.0 to 4.0 and increase start distance from 200 to 500.

4. **Stained glass translucency:** Each pane is a translucent material. Limit visible panes to 8-12 at once. Use forward-rendered translucency for glass if sort issues arise.

5. **VRAM (16GB):** Sufficient for 1080p with Lumen + Nanite. Set texture streaming pool to 8-10GB (`r.Streaming.PoolSize=8192`) to avoid hitches.

6. **C: drive full (23GB):** DDC redirected to K:. All Megascans downloads go to K:. Intermediate junctioned to K:. Should not impact runtime performance but monitor.

### 9.3 Fallback Quality Settings (if 60 FPS not achieved)

| Level | Lumen | Resolution | Nanite Tris | Fog | Target FPS |
|-------|-------|-----------|------------|-----|-----------|
| High | Software, Q2 | 1080p | 2M | 0.015 density | 60 |
| Medium | Software, Q1 | 1080p | 1.5M | 0.01 density | 60 |
| Low | SSR+SSAO only | 900p | 1M | 0.005 density | 60 |

### 9.4 Key Console Variables for Optimization
```
r.Lumen.HardwareRayTracing=False    # RDNA2 has no DXR
r.Lumen.ScreenSpaceGI=True          # Cheaper GI fallback
r.Streaming.PoolSize=8192           # 8GB texture pool
r.Shadow.MaxResolution=4096         # VSM shadow resolution
r.VolumetricFog.GridSizeZ=32        # Reduce if fog is expensive
r.Nanite.MaxPixelsPerComponent=1.0   # Nanite pixel budget
```

---

## 10. IMPLEMENTATION ROADMAP

### Phase 1: Foundation (REQUIRES UE5.8 EDITOR RESTART + GUI)
1. ✅ MegascansPlugin ENABLED in .uproject — DONE (takes effect on restart)
2. ✅ DDC redirected to K: — DONE (DefaultEngine.ini)
3. ✅ Render config set — DONE (Lumen, Nanite, VSM, volumetric fog)
4. ⬜ Restart UE5.8 for MegascansPlugin activation
5. ⬜ Login Quixel Bridge (Window → Quixel Bridge, Epic Games login)
6. ⬜ Download Gothic/cathedral Megascans assets to K: drive
7. ⬜ Import to Content/Environments/Solara/
8. ⬜ Enable Nanite on all imported meshes

### Phase 2: Lighting & Atmosphere (Python script ready — needs editor GUI or SendKeys)
1. ⬜ Open Lvl_SolaraCathedral in editor
2. ⬜ Run solara_cathedral_lighting_v2.py (sets up Directional Light, Sky Light, ExpHeightFog, PostProcess)
   - Script location: Content/Python/solara_cathedral_lighting_v2.py
   - Also copied to: Content/Python/solara.py (shorter path for console)
3. ⬜ Verify lighting in viewport (god rays, warm/cool contrast)
4. ⬜ Adjust fog density if too thick/thin
5. ⬜ Test Lumen GI response (bounced light from stained glass)

### Phase 3: Cathedral Dressing
1. ⬜ Apply proper materials to SOLARA_CATHEDRAL_AAA mesh (currently default Interchange opaque)
2. ⬜ Replace/augment with Megascans Gothic assets
3. ⬜ Add environmental storytelling props (pews, prayer books, campfire, etc.)
4. ⬜ Create stained glass emissive material (blue/violet glow)
5. ⬜ Create purple energy vein material (emissive, pulsing)
6. ⬜ Place Grace Site checkpoint
7. ⬜ Set up Chaos destruction for boss arena pillars

### Phase 4: Optimization & Validation
1. ⬜ GPU Visualizer profiling (stat gpu)
2. ⬜ Nanite stats (Nanite visualization)
3. ⬜ Lumen stats (Lumen visualization)
4. ⬜ Test at 1080p 60 FPS target on RX 6800
5. ⬜ Adjust fog density, particle counts, Lumen quality as needed
6. ⬜ Capture in-engine screenshots
7. ⬜ Report verified FPS numbers

### Phase 5: World Partition & Expansion
1. ⬜ Convert to World Partition level
2. ⬜ Set up Data Layers (Exterior, Nave, BossArena, CosmicVoid)
3. ⬜ Test streaming behavior
4. ⬜ Plan exterior approach zone
5. ⬜ Plan full Solara region connection points

---

## 11. PYTHON SCRIPTS (READY TO RUN IN UE5.8 EDITOR)

### 11.1 Lighting Setup Script
- Location: Content/Python/solara_cathedral_lighting_v2.py
- Also at: Content/Python/solara.py (shorter path)
- Sets up: Directional Light (warm amber), Sky Light (violet), ExponentialHeightFog (volumetric), PostProcess Volume (ACES, bloom, vignette)
- Usage in UE5.8 console: `py content/python/solara.py`
- Note: Must be run from the UE5.8 editor console (backtick key). SendKeys from cron is unreliable.

### 11.2 Material Assignment Script (TO BE WRITTEN)
- Will assign Megascans materials to SOLARA_CATHEDRAL_AAA sections
- Needs Megascans assets imported first
- Will create material instances for Lumen compatibility

---

## 12. HONEST STATUS

### What's VERIFIED working:
- ✅ UE5.8 running (PID 223644, 1.35GB RAM)
- ✅ Remote Control API on port 30010 (asset search confirmed)
- ✅ Render config in DefaultEngine.ini (Lumen, Nanite, VSM, volumetric fog)
- ✅ MegascansPlugin in .uproject (enabled, pending restart)
- ✅ SOLARA_CATHEDRAL_AAA mesh with Nanite (672K tris)
- ✅ C++ level controller with 5 zone definitions
- ✅ Design document v1 (comprehensive) — now updated to v2
- ✅ Python lighting script ready to run

### What BLOCKS completion (cannot do from cron):
1. **Quixel Bridge login** — needs interactive Epic Games login in the UE5.8 editor GUI
2. **Megascans asset download** — needs Bridge GUI session
3. **Level dressing** — placing Megascans assets in the level requires the editor viewport
4. **Material assignment** — applying textures to the cathedral mesh requires the material editor
5. **SendKeys to UE5.8** — unreliable from cron (timing issues with Shift key, paste not registering)
6. **In-engine screenshots** — cannot reliably capture from cron without SendKeys working

### What Jamil needs to do (interactive session):
1. Restart UE5.8 (for MegascansPlugin to activate)
2. Open Quixel Bridge (Window → Quixel Bridge) and login with Epic account
3. Download Gothic/cathedral Megascans assets to K: drive
4. Open Lvl_SolaraCathedral in the editor
5. Run `py content/python/solara.py` in the console (backtick) to set up lighting
6. Place Megascans assets and apply materials
7. Capture screenshots and report FPS (stat gpu)

---

*Last updated: November 9, 2026 — World Director, Momentum Labs*
*Verified via: Remote Control API asset search, DefaultEngine.ini review, .uproject review, UE5.8 log analysis*
